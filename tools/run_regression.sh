#!/usr/bin/env bash
set -euo pipefail

# Runs Lama regression tests by comparing the reference lamac interpreter output
# against the current lama_sm_interpreter executable on compiled bytecode.
#
# Usage:
#   tools/run_regression.sh [REGRESSION_DIR] [INTERPRETER_PATH] [--keep-going|-k]
#
# - REGRESSION_DIR: Path to directory containing testNNN.lama and testNNN.input files.
#                   Defaults to ~/Lama/regression
# - INTERPRETER_PATH: Path to the built lama_sm_interpreter executable.
#                     Defaults to <repo_root>/cmake-build-debug/lama_sm_interpreter
# - --keep-going|-k: Do not stop on per-test failure; continue running the rest and show a summary at the end.
#
# Behavior:
# - For each *.lama in REGRESSION_DIR, expects an optional matching *.input.
# - Runs: lamac -i file.lama < file.input, then removes all occurrences of the
#         substring " > " from its output.
# - Compiles: lamac -b file.lama to produce file.bc.
# - Runs: lama_sm_interpreter -i file.bc < file.input.
# - Compares the two outputs; on mismatch prints both and exits with non-zero status.

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

# Helper to print to stderr
err() { printf '%s\n' "$*" >&2; }

# Parse arguments flexibly: accept --keep-going/-k anywhere.
# First non-option is REGRESSION_DIR, second non-option is INTERPRETER_PATH.
REG_DIR=""
INTERP_PATH=""
KEEP_GOING=false
for arg in "$@"; do
  case "$arg" in
    --keep-going|-k)
      KEEP_GOING=true
      ;;
    --help|-h)
      cat <<EOF
Usage: tools/run_regression.sh [REGRESSION_DIR] [INTERPRETER_PATH] [--keep-going|-k]
- REGRESSION_DIR: Path to tests (default: $HOME/Lama/regression)
- INTERPRETER_PATH: Path to lama_sm_interpreter (default: ${REPO_ROOT}/cmake-build-debug/lama_sm_interpreter)
- --keep-going|-k: Continue after failures. Options may appear anywhere.
EOF
      exit 0
      ;;
    *)
      if [[ -z "$REG_DIR" ]]; then
        REG_DIR="$arg"
      elif [[ -z "$INTERP_PATH" ]]; then
        INTERP_PATH="$arg"
      else
        err "Warning: ignoring extra argument '$arg'"
      fi
      ;;
  esac
done

# Apply defaults if not set via args
REG_DIR="${REG_DIR:-$HOME/Lama/regression}"
INTERP_PATH="${INTERP_PATH:-${REPO_ROOT}/cmake-build-debug/lama_sm_interpreter}"

# Basic checks
if ! command -v lamac >/dev/null 2>&1; then
  err "Error: 'lamac' not found in PATH. Please install it or add to PATH."; exit 127;
fi
if [[ ! -x "${INTERP_PATH}" ]]; then
  err "Error: lama_sm_interpreter not found or not executable at: ${INTERP_PATH}";
  err "Hint: build it in your active CLion/CMake profile or pass its path as the 2nd argument."; exit 127;
fi
if [[ ! -d "${REG_DIR}" ]]; then
  err "Error: Regression directory does not exist: ${REG_DIR}"; exit 2;
fi

cd -- "${REG_DIR}"
shopt -s nullglob
lamas=( *.lama )
shopt -u nullglob

if (( ${#lamas[@]} == 0 )); then
  err "No .lama files found in ${REG_DIR}"; exit 0;
fi

# Work in a temporary area for outputs to avoid interfering with files
TMP_BASE="$(mktemp -d -t lama-reg-XXXXXX)"
cleanup() { rm -rf -- "${TMP_BASE}"; }
#trap cleanup EXIT

pass_count=0
fail_count=0
skip_count=0

for lama_file in "${lamas[@]}"; do
  base="${lama_file%.lama}"
  input_file="${base}.input"

  echo $base

  # Determine if input redirection should be used
  use_input=false
  if [[ -f "${input_file}" ]]; then
    use_input=true
  fi

  ref_out="${TMP_BASE}/${base}.ref.out"
  sm_out="${TMP_BASE}/${base}.sm.out"

  # 1) Run reference interpreter (lamac -i) and sanitize output
  if $use_input; then
    if ! lamac -i "${lama_file}" <"${input_file}" >"${ref_out}.raw" 2>"${ref_out}.stderr"; then
      printf 'SKIP: %s (lamac -i failed)\n' "${base}"
      skip_count=$((skip_count + 1))
      continue
    fi
  else
    if ! lamac -i "${lama_file}" >"${ref_out}.raw" 2>"${ref_out}.stderr"; then
      printf 'SKIP: %s (lamac -i failed)\n' "${base}"
      skip_count=$((skip_count + 1))
      continue
    fi
  fi
  # Remove all occurrences of the substring " > "
  sed 's/ > //g' "${ref_out}.raw" >"${ref_out}"

  # 2) Compile to bytecode with lamac -b
  if ! lamac -b "${lama_file}" >"${TMP_BASE}/${base}.build.log" 2>&1; then
    printf 'SKIP: %s (lamac -b failed)\n' "${base}"
    skip_count=$((skip_count + 1))
    continue
  fi
  bc_file="${base}.bc"
  if [[ ! -f "${bc_file}" ]]; then
    printf 'SKIP: %s (bytecode not found)\n' "${base}"
    skip_count=$((skip_count + 1))
    continue
  fi

  # 3) Run our stack machine interpreter on the bytecode
  if $use_input; then
    if ! "${INTERP_PATH}" -i "${bc_file}" <"${input_file}" >"${sm_out}.raw" 2>"${sm_out}.stderr"; then
      if grep -qF 'not implemented' "${sm_out}.stderr"; then
        printf 'SKIP: %s (not implemented)\n' "${base}"
        cat "${sm_out}.stderr"
        skip_count=$((skip_count + 1))
        continue
      fi
      err "lama_sm_interpreter failed for ${bc_file}. See ${sm_out}.stderr"
      cat "${sm_out}.stderr"
      fail_count=$((fail_count + 1))
      if ! $KEEP_GOING; then exit 1; else continue; fi
    fi
  else
    if ! "${INTERP_PATH}" -i "${bc_file}" >"${sm_out}.raw" 2>"${sm_out}.stderr"; then
      if grep -qF 'not implemented' "${sm_out}.stderr"; then
        printf 'SKIP: %s (not implemented)\n' "${base}"
        cat "${sm_out}.stderr"
        skip_count=$((skip_count + 1))
        continue
      fi
      err "lama_sm_interpreter failed for ${bc_file}. See ${sm_out}.stderr"
      cat "${sm_out}.stderr"
      fail_count=$((fail_count + 1))
      if ! $KEEP_GOING; then exit 1; else continue; fi
    fi
  fi

  sed 's/> //g' "${sm_out}.raw" >"${sm_out}"

  # 4) Compare outputs
  if ! cmp -s "${ref_out}" "${sm_out}"; then
    err "Mismatch detected in ${base}:"
    err "---- lamac (sanitized) output ----"
    cat -- "${ref_out}" >&2
    err "---- lama_sm_interpreter output ----"
    cat -- "${sm_out}" >&2
    fail_count=$((fail_count + 1))
    if ! $KEEP_GOING; then exit 1; fi
  else
    printf 'OK: %s\n' "${base}"
    pass_count=$((pass_count + 1))
  fi

done

# Final summary
total=$((pass_count + fail_count))
printf '\nSummary: passed=%d, failed=%d, skipped=%d, total=%d\n' "${pass_count}" "${fail_count}" "${skip_count}" "${total}"
if (( fail_count > 0 )); then exit 1; else exit 0; fi
