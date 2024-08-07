# Upsy-Downsy

Entry for LowRezJam 2024: https://itch.io/jam/lowrezjam-2024
Theme "You Are The Environment".

Also a trial of my new and incomplete platform Pebble: https://github.com/aksommerville/pebble

Help the rabbit reach his carrot by raising or lowering columns of earth.

## Notes

Try a 10x10 grid of 6x6-pixel tiles, leaves us 4 pixels for a status bar.

## TODO

- [x] Editor is kind of painful.
- - [x] Universal fallback: Show full text at all times.
- - [x] Ensure we're able to grab floor when it's at zero.
- - [x] Widen things up to the edges.
- [ ] Final scenes. Finish by 9 August.
- - [x] Start at the end and progress downward, just while designing.
- - [ ] 1: Make a bunch, like at least 30. ...30 is a lot. Got 16 so far.
- - [ ] 2: Sort by difficulty.
- - [ ] 3: Eliminate ones too similar to others, or that I don't like after playing.
- - [ ] 4: Repack IDs.
- [ ] Try drawing the crocodile a little shorter.
- [x] Try having the rabbit walk on two instead of three. scene:13 would work better
- [ ] Period and phase for flamethrowers.
- [ ] Vertical (upward) flamethrowers. Can we use the same object, just add a height attribute?
- [x] Check for walkability if it was declined due to hammer -- hammer won't be there forever. Or just don't count the hammer.
- - Apparent at scene:4
- [x] Observed hammer smash putting gore in the wrong place, scene:8
- [x] Moving the crocodile feels wrong, like, which column do I use? Should be either of the two columns most below him.
- [ ] Calibrate scorekeeping, ensure 10k is not reachable.
- [ ] Revisit margins, I've only made them worse.
- [ ] Try a cheerier color scheme.
- [ ] Pause on blur, or at least halt audio.
- [ ] Decoration and verbiage for Itch page.
- [x] Persistence isn't working on itch. Arrange a local server and test.
- - Forgot, you need "version" or "persistKey" to make that work in web.
