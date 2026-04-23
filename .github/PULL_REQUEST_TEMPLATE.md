## Description
<!-- Provide a brief, concise summary of the changes introduced in this PR. Explain the 'why' alongside the 'what'. -->


## Related Issue / Story
<!-- Link to the Trello card or Story ID this PR addresses. -->
- **Story ID:**
- **Trello Card:**

## Changes Made
<!-- Detail the key structural, architectural, or behavioral changes. -->
-
-

## Architectural & Technical Constraints Check
<!-- This is a standard C17 project with file-based persistence. Ensure the following constraints are met. -->
- [ ] **Standard C17:** Code strictly adheres to C17 standard and compiles cleanly without warnings.
- [ ] **Data Persistence:** Immediate binary file write operations are executed after any array mutation to prevent data loss.
- [ ] **Memory Safety:** Safe input handling is implemented with bounds checking to prevent buffer overflows.
- [ ] **Clean Architecture:** Data is passed cleanly via the `AppDatabase` pointer. No unnecessary global variables are introduced.
- [ ] **Code Quality:** Code is formatted with `.clang-format` and passes `.clang-tidy`.

## Acceptance Criteria
<!-- Verify that all specific requirements defined in the Story are fulfilled. -->
- [ ] All acceptance criteria from the assigned story are fully met.
- [ ] Role and permission boundaries are accurately enforced (BCN vs. Member).
- [ ] Appropriate error handling and input validation messages (e.g., `[LOI]`, `[OK]`) are displayed.

## Testing & Verification
<!-- Describe how you verified these changes. Include any manual CLI tests, compile logs, or edge-case testing. -->
- [ ] Clean build completed successfully.
- [ ] Manual end-to-end CLI flow tested.
- [ ] Edge cases (e.g., missing data files, maximum capacity reached, invalid inputs) verified.

**Test Notes:**
```text
(Add commands run, scenarios tested, or any known limitations here)
```

## Reviewer Focus
<!-- Call out the most critical parts of the code for the reviewer (e.g., complex business logic, struct alignment, or binary I/O). -->
-
