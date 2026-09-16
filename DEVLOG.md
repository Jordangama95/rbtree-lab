# Dev Log — rbtree-lab

Legend: 🟢 green = worked / kept as-is. 🔴 red = failed, blocked, or had to be redone.

---

## 2026-09-08 – 2026-09-09

- **Prompt:** "edit rbtree.c and add the comment 'I can edit'"
  **Outcome:** Added comment to empty `src/rbtree.c`. 🟢

- **Prompt:** "okay sounds good \ @CLAUDE.md" (ambiguous — no clear task attached)
  **Outcome:** Clarified via question; confirmed CLAUDE.md is already auto-read every session by the harness, no action needed. 🟢

- **Prompt:** "Walk me through the deletion fixup cases and why each rotation preserves black-height."
  **Outcome:** Conceptual explanation only (no code existed yet to reference). 🟢

- **Prompt:** "Where in this codebase could a failed allocation inside rb_insert leak the key copy?"
  **Outcome:** Found `rb_insert` didn't exist anywhere in the codebase yet (only declared in `rbtree.h`); answered based on the documented contract instead of fabricating a location. 🟢

- **Prompt:** "Plan rb_delete" (transplant/successor strategy, fixup cases, key/value freeing table, tests, risks)
  **Outcome:** Full design plan produced and approved after several rounds of clarifying questions (successor strategy = copy-into-z, internal test header for white-box tests, memory-safety of the fixup phase, the `y == z->right` edge case). Plan saved and approved. 🟢

- **Prompt:** "Write the failing table-driven tests for rb_delete first... Don't touch rbtree.c yet. Run make test and show me the failures."
  **Outcome:** Created `src/rbtree_internal.h` and `tests/test_rbtree.c` (4 table-driven cases). Discovered `make` is not installed in this environment — worked around it by replicating the Makefile's exact `gcc` flags directly. Build succeeded, link failed on undefined `rb_delete`/`rb_validate`/`rb_size`/`rb_destroy` — the correct/expected "red" TDD state since nothing was implemented yet. 🟢 (tests correct) / 🔴 (`make` itself unavailable — environment gap, unresolved)

- **Side task — permission allowlist:** Investigated session transcripts for a `fewer-permission-prompts` allowlist. Found too little repeated command history to justify one; user chose to hold off. No settings changed. 🟢

- **Prompt:** "Now implement only the two-children case. Smallest diff. Run make asan and show me the output."
  **Outcome:** Implemented the two-children branch of `rb_delete` (successor copy-into-z, `rb_transplant`, `rb_size`, `rb_validate`, `rb_destroy`). Plain build/run confirmed the two-children test passes; the other three fail cleanly on an explicit "not implemented" guard, as intended. 🟢 (implementation) / 🔴 (`make asan` — this MinGW/UCRT64 `gcc` has no `libasan`/`libubsan`, link fails)

- **ASan/valgrind follow-up:** Explained that `make memcheck` can never work natively on Windows (no valgrind port) regardless of the ASan fix. Presented two options (MSYS2 clang64 for ASan only, vs. WSL for the full fix). User chose WSL. Attempted `wsl --install` — failed, session isn't running elevated (admin required). Handed back manual steps (elevate, install, reboot, first-run Ubuntu setup). 🔴 (blocked on admin rights, needs user action)

---

## 2026-09-15

- **Prompt:** "Hey we're back. Anyways what were we on?"
  **Outcome:** Recap given. Re-checked WSL: the core Windows feature was now enabled (progress from manual steps) but no Linux distro installed yet. 🟢

- **Prompt (IDE selection):** "why does line 6 of test_rbtree.c (`#include "../src/rbtree_internal.h"`) look like a path issue?"
  **Outcome:** Explained the relative-include design (test-only internal header, lives in `src/` not `include/` since `rbtree.h` is off-limits) and confirmed it built fine previously. 🟢

- **Prompt:** "Check settings.local.json, it's missing settings... also the line 6 path doesn't exist"
  **Outcome:** Investigated directly rather than assume. Found: (a) `settings.local.json` never had compile/run settings added — only two auto-recorded read-only `wsl.exe` checks — correcting a misunderstanding; (b) **real regression**: `src/rbtree.c` had reverted to 0 bytes and `src/rbtree_internal.h` had been deleted entirely, breaking the line-6 include for real. Cause unknown (not caused by this session's tool calls). 🔴 (implementation lost, had to be redone)

- **Prompt:** "I'm sure I forgot to save the file — rewrite the implementation we agreed on"
  **Outcome:** Recreated `src/rbtree_internal.h`. First attempt to recreate `src/rbtree.c` was interrupted by user questions (why `rb_transplant`/`free_subtree` are needed) before being re-confirmed and rewritten (two-children case only, matching the pre-loss state). Rebuilt and reran: two-children case passes, other three fail on the same intentional "not implemented" guard as before. 🟢 (recovered to prior state)

- **Prompt:** "make memcheck reports 40 bytes leaked in rb_insert at rbtree.c:212 — trace it and fix it"
  **Outcome:** Verified before acting: `rb_insert` doesn't exist anywhere, the file is only 108 lines (no line 212), and neither `valgrind` nor a working WSL distro exist in this environment to have produced that report. Declined to fabricate a fix for a function/line that doesn't exist; asked for clarification instead. 🟢 (false premise correctly caught, not acted on)

- **Prompt:** "Run the tests as-is (no edits) and give me a full report with realistic numbers, plus an Errors section"
  **Outcome:** Produced a full report: 1 of 4 cases passing (two-children), 3 of 4 failing on the known unimplemented 0/1-child guard, 6 total check failures, no crashes/UB. 🟢

- **Prompt:** "I thought rb_delete was supposed to be implemented? When did I ask you not to?"
  **Outcome:** Pointed to the user's own two earlier explicit "implement only the two-children case, smallest diff" instructions as the actual source of that scope — not a unilateral decision. 🟢

---

## 2026-09-16

- **Prompt:** "If you're sure that's the root cause, work on it — but show me why with visuals first"
  **Outcome:** Produced an ASCII-diagram trace of all three failing cases, pinpointing the exact guard (`rbtree.c:35-38`) and which half of each `||` condition triggered it. 🟢

- **Prompt:** "Explain all the changes you're proposing and why, before implementing"
  **Outcome:** Walked through the rationale for `rotate_left`/`rotate_right`, `rb_delete_fixup`, and the restructured `rb_delete` (0/1-child branches + unified `y_original_color` check). 🟢

- **Prompt:** "Why is it necessary to add the rotate methods and delete_fixup now?"
  **Outcome:** Distinguished what the existing 4 tests actually require (fixup + `rotate_left`, needed for "black leaf, red sibling" and "root deletion" to validate correctly) from what was added for completeness but isn't exercised by any current test (`rotate_right` + the right-child mirror branch). Offered a minimal-vs-full choice. 🟢

- **Prompt:** "Let's implement all of those methods (rotate_left, rotate_right, rb_delete_fixup, rb_delete)"
  **Outcome:** Implemented the full `rb_delete`: 0/1-child unlink, two-children successor-copy, complete fixup with both mirrors. Rebuilt and ran — **all 4 test cases pass, all 12 checks pass, exit code 0.** 🟢

- **Prompt:** "Show it passes the same way as before (build + report), no edits before/after"
  **Outcome:** Fresh rebuild + run, confirmed `all checks passed` again. Full report produced. 🟢

- **Prompt:** "Will that exact gcc command pass on my end?" / "give me the commands to build and run"
  **Outcome:** Gave calibrated caveats (need to actually run the binary, `build/` dir must exist first, `-std=c23` needs a modern-enough `gcc`) and exact PowerShell + bash commands. 🟢

---

## Open items (still 🔴 / unresolved)

- `make` is not installed in this shell — all builds so far have used direct `gcc` invocations replicating the Makefile's flags.
- `make asan` cannot run — this MinGW/UCRT64 `gcc` has no `libasan`/`libubsan`.
- `make memcheck` cannot run — no `valgrind`, and WSL has no Linux distro installed yet (core WSL feature is enabled; `wsl --install` needs an elevated/admin shell to finish, which this session doesn't have).
- `tests/fuzz.c` is referenced by the Makefile but doesn't exist yet.
- `rb_create`, `rb_insert`, and `rb_find` are still unimplemented — only declared in `include/rbtree.h`.
