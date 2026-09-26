# show_credits — current focused finding

The accepted body is 654 bytes against the historical 634 and remains `DIFFER`.
The first raw mismatch at +26 is an unresolved `_screen` address, not the
control-flow divergence. The first drawing sequence and x87 volume setup agree
in structure. The candidate then duplicates the Esc/cycle guard after the
inner wait and before `vol -= vol_step`; the historical loop subtracts first
and returns to one guard block.

Scratch rounds 1–3 measured the baseline and tested close-button guard order,
an explicit loop label, `for` update clauses, and a guarded `do` loop. The
equivalent forms compile back to the 654-byte output; guarded `do` is 658 bytes.
No candidate is promotable. The prior read-only archive analysis at
`../icytower_recon/docs/attempts/research-luna-show-credits/README.md` explains
the mechanism: GCC 4.4.1 loop-header copying clones the guard and a later sink
pass moves the locally dead subtraction. The follow-up note there found no
historically grounded body-only way around those passes. Further work should
seek build/source evidence that changes the optimizer input, not repeat loop
spellings or inject volatile/attributes.
