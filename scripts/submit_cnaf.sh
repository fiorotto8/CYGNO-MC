#!/bin/bash
# Submit one CNAF job (HTCondor via cygno_htc / mycondor subber.sh) per Geant4 macro.
#
# The macros are prepared beforehand, e.g. with
#   python3 scripts/submit_jobs_lngs.py -d -m <template> -f <List>Activity.txt -e 100000 \
#       --tag <tag> --codedir <CYGNO-MC> --builddir <build> --outdir <dir>
# which writes one macro per job (isotope, events, output name, random seeds)
# into <dir>/pbs_workdir/<tag>/ without submitting anything.
#
# For each macro this script creates a job folder (macro, CYGNO, geometry archive,
# run_mc.sh), fills a copy of subber.sh and sources it from the Launch directory,
# as in the single-job procedure of the README.
set -euo pipefail

usage() {
  cat <<EOF
Usage: $(basename "$0") -m MACRODIR -s SUBBER -x CYGNO -g GEO_TGZ -T TAG [options]

  -m DIR    directory containing the job macros (*.mac)
  -s FILE   subber.sh template, inside Launch/Sub_folder/ (from /cvmfs/sft-cygno.infn.it/config/templates/)
  -x FILE   CYGNO executable
  -g FILE   geometry archive, e.g. geo_v4.tar.gz containing cygno04_v4_ASCII/
  -T TAG    output path in the bucket; must start with From_queue/
  -w DIR    folder for the job directories (default: ./cnaf_jobs)
  -c N      CPUs and Geant4 threads per job (default: 2)
  -p SEC    pause between submissions (default: 1)
  -k        prepare the job folders and subber copies only, do not submit
  -h        show this help

Environment: LIST_SEP separates the entries of OTHERFILE and FILENAMES (default: space).
Submitted job names are appended to <work dir>/submitted.txt; rerunning skips them.
EOF
}

MACRODIR="" SUBBER="" CYGNO_BIN="" GEO_TGZ="" OUT_TAG=""
WORK="./cnaf_jobs" CPUS=2 PAUSE=1 DRYRUN=0
LIST_SEP="${LIST_SEP:- }"

while getopts "m:s:x:g:T:w:c:p:kh" opt; do
  case $opt in
    m) MACRODIR=$OPTARG ;;
    s) SUBBER=$OPTARG ;;
    x) CYGNO_BIN=$OPTARG ;;
    g) GEO_TGZ=$OPTARG ;;
    T) OUT_TAG=$OPTARG ;;
    w) WORK=$OPTARG ;;
    c) CPUS=$OPTARG ;;
    p) PAUSE=$OPTARG ;;
    k) DRYRUN=1 ;;
    h) usage; exit 0 ;;
    *) usage; exit 1 ;;
  esac
done

die() { echo "ERROR: $*" >&2; exit 1; }

[[ -n $MACRODIR && -n $SUBBER && -n $CYGNO_BIN && -n $GEO_TGZ && -n $OUT_TAG ]] || { usage; exit 1; }
[[ -d $MACRODIR ]] || die "macro directory $MACRODIR not found"
[[ -f $SUBBER ]] || die "subber template $SUBBER not found"
[[ -x $CYGNO_BIN ]] || die "CYGNO executable $CYGNO_BIN not found or not executable"
[[ -f $GEO_TGZ ]] || die "geometry archive $GEO_TGZ not found"
[[ $OUT_TAG == From_queue/* ]] || die "TAG must start with From_queue/ (other bucket folders are deleted)"
[[ $CPUS =~ ^[0-9]+$ && $CPUS -ge 1 && $CPUS -le 32 ]] || die "CPUs must be between 1 and 32"

abspath() { (cd "$(dirname "$1")" && echo "$PWD/$(basename "$1")"); }
SUBBER=$(abspath "$SUBBER")
CYGNO_BIN=$(abspath "$CYGNO_BIN")
GEO_TGZ=$(abspath "$GEO_TGZ")
SUBDIR=$(dirname "$SUBBER")             # Launch/Sub_folder
LAUNCH=$(dirname "$SUBDIR")             # Launch
SUBREL=$(basename "$SUBDIR")
CODEDIR=$(cd "$(dirname "$0")/.." && pwd)
GEO_NAME=$(basename "$GEO_TGZ")
GEO_DIR=$(tar -tzf "$GEO_TGZ" | awk -F/ 'NR==1{d=$1} END{print d}')   # read all: head would SIGPIPE tar under pipefail
[[ -n $GEO_DIR ]] || die "cannot read the top folder of $GEO_TGZ"

# Every variable this script fills must exist in the template.
for v in EXECUTEPATH EXECUTE CPUs OTHERFILE FILENAMES BATCHNAME TAG; do
  grep -qE "^[[:space:]]*(export[[:space:]]+)?$v=" "$SUBBER" \
    || die "variable $v not found in $SUBBER; adapt set_var calls to the template"
done

# Replace the assignment of variable $2 in file $1 with value $3.
set_var() {
  local value=${3//\\/\\\\}
  value=${value//&/\\&}
  sed -i -E "s|^([[:space:]]*(export[[:space:]]+)?$2=).*|\1\"${value}\"|" "$1"
}

join() { local IFS="$LIST_SEP"; echo "$*"; }

mkdir -p "$WORK"
WORK=$(cd "$WORK" && pwd)
touch "$WORK/submitted.txt"

shopt -s nullglob
macros=("$MACRODIR"/*.mac)
[[ ${#macros[@]} -gt 0 ]] || die "no .mac files in $MACRODIR"
echo "${#macros[@]} macros, geometry $GEO_DIR, $CPUS CPUs, output $OUT_TAG"

n_new=0
for mac in "${macros[@]}"; do
  name=$(basename "$mac" .mac)
  if grep -qxF "$name" "$WORK/submitted.txt"; then
    echo "skip $name (already submitted)"; continue
  fi
  outfile=$(awk '$1=="/CYGNO/outfile"{print $2}' "$mac" | tail -1)
  [[ -n $outfile ]] || die "$mac has no /CYGNO/outfile"
  grep -q "^/random/setSeeds" "$mac" || die "$mac has no /random/setSeeds; jobs would repeat the same events"

  jobdir="$WORK/$name"
  mkdir -p "$jobdir"
  cp "$mac" "$jobdir/run.mac"
  ln -sf "$CYGNO_BIN" "$jobdir/CYGNO"
  ln -sf "$GEO_TGZ" "$jobdir/$GEO_NAME"
  inputs=("$jobdir/CYGNO" "$jobdir/run.mac" "$jobdir/$GEO_NAME")
  if grep -q "input_distribution" "$mac"; then
    ln -sfn "$CODEDIR/input_distribution" "$jobdir/input_distribution"
    inputs+=("$jobdir/input_distribution")
  fi

  cat > "$jobdir/run_mc.sh" <<EOF
#!/bin/bash
set -e
tar -xzf $GEO_NAME
export CYGNO_GEOMETRY_PATH="\$PWD/$GEO_DIR"
export G4FORCENUMBEROFTHREADS=$CPUS
exec ./CYGNO run.mac > $name.log 2>&1
EOF
  chmod +x "$jobdir/run_mc.sh"

  sub="$SUBDIR/subber_$name.sh"
  cp "$SUBBER" "$sub"
  set_var "$sub" EXECUTEPATH "$jobdir"
  set_var "$sub" EXECUTE "run_mc.sh"
  set_var "$sub" CPUs "$CPUS"
  set_var "$sub" OTHERFILE "$(join "${inputs[@]}")"
  set_var "$sub" FILENAMES "$(join "$outfile.root" "$name.log")"
  set_var "$sub" BATCHNAME "$name"
  set_var "$sub" TAG "$OUT_TAG"

  if [[ $DRYRUN -eq 1 ]]; then
    echo "prepared $name -> $sub"; continue
  fi
  echo "submitting $name"
  (set +euo pipefail; cd "$LAUNCH" && source "$SUBREL/subber_$name.sh") > "$jobdir/submit.log" 2>&1 \
    || die "submission of $name failed, see $jobdir/submit.log"
  echo "$name" >> "$WORK/submitted.txt"
  n_new=$((n_new + 1))
  sleep "$PAUSE"
done

echo "done: $n_new submitted now, $(wc -l < "$WORK/submitted.txt") in total ($WORK/submitted.txt)"
