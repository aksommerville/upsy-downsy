# Upsy-Downsy Scene Format

Usually I'd do separate text and binary formats, but there's no need. They'll be tiny either way.

Line-oriented text.
'#' begins a line comment, start of line only.

Line begins with a keyword, followed by arguments, which can only be decimal integers.

| Keyword     | Arguments |
|-------------|-----------|
| dirt        | Ten integers in 0..10. |
| rabbit      | `X Y` 0..9 |
| carrot      | `X Y` 0..9 |
| song        | `SONGID` |
| hammer      | `X W PERIOD PHASE` Times in ms. Min PERIOD 1000. |
| crocodile   | `X Y` |
| hawk        | (none) |
