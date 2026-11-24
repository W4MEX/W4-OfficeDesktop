#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Euro-Office contributors
# SPDX-License-Identifier: AGPL-3.0-only

# Usage: test-plan.sh <build.yml>. Runs the plan step for each event and checks the brands,
# then checks that ci-result runs on every run and only passes when all needed jobs succeeded.
set -euo pipefail
wf=$1; dir=$(mktemp -d)
yq '.jobs.plan.steps[0].run' "$wf" > "$dir/run.sh"
export BRANDS=$(yq '.jobs.plan.steps[0].env.BRANDS' "$wf")
fail=0
check() {
  GITHUB_EVENT_NAME=$1 GITHUB_OUTPUT=$dir/out bash "$dir/run.sh"
  out=$(sed -n 's/^brands=//p' "$dir/out")
  rm -f "$dir/out"
  if jq -e 'length == 1 and (.[0] | .name == "w4-office" and .company_name == "W4" and .product_name == "W4 Office" and .extra_bake_files_common == "./w4/branding.hcl")' <<<"$out" >/dev/null; then
    echo "ok   $1 selects the W4 Office profile"
  else
    echo "FAIL $1 returned an unexpected profile: $out"
    fail=1
  fi
}

check pull_request
check push
check workflow_dispatch

# ci-result: a skipped required check counts as passed, so the job itself must never skip.
cond=$(yq '.jobs.ci-result.if' "$wf")
if [[ "$cond" == "always()" ]]; then echo "ok   ci-result if: $cond"; else echo "FAIL ci-result if: $cond, want always()"; fail=1; fi
yq '.jobs.ci-result.steps[0].run' "$wf" > "$dir/result.sh"
result() { # results expected
  if RESULTS=$1 bash "$dir/result.sh" >/dev/null; then got=pass; else got=fail; fi
  if [[ "$got" == "$2" ]]; then echo "ok   ci-result [$1] -> $got"; else echo "FAIL ci-result [$1] -> $got, want $2"; fail=1; fi
}
result "success success success" pass
result "success failure success" fail
result "success cancelled success" fail
result "success skipped success" fail
exit $fail
