# axotext

**by [axollyon](https://axollyon.com/)**

**based on [HackerSM64 v2.1](https://github.com/HackerN64/HackerSM64)**

### note: if you were using axotext prior to august 13, 2023, you now need to call `axotext_render()` after you're finished printing!

## to use:
- make an `AxotextFont` [(example)](https://github.com/axollyon/axotext/blob/axotext/bin/axotext/comicsans.inc.c) with the help of [TTF2AxoText](https://github.com/Fallden4/TTF2AxoText) by Fallden4
- include your font in either `bin/segment2.c` (segment 2, always loaded) or `levels/<any level>/leveldata.c` (segment 7, only loaded in that level)
- extern your font in either `game/segment2.h` or `game/segment7.h`
- include both `"segment2.h"`/`"segment7.h"` and `"axotext.h"` in the code file where you want to print your text
- declare an `AxotextParams` in your function just before you want to print your text
  - like this:
    - ```c
      AxotextParams params = {
        &comicsans, // font name (remember the '&')
        16, // font size (in pixels on the framebuffer)
        16, // line height (in pixels on the framebuffer)
        AXOTEXT_ALIGN_CENTER, // text alignment (AXOTEXT_ALIGN_LEFT, AXOTEXT_ALIGN_CENTER, and AXOTEXT_ALIGN_RIGHT)
        255, // red
        255, // green
        255, // blue
        255  // alpha
      };
      ```
- call `axotext_print`
  - like this:
    - ```c
      axotext_print(
        160, // x position (in pixels on the framebuffer, where 0 is the left edge)
        120, // y position (in pixels on the framebuffer, where 0 is the bottom edge)
        &params, // the params you defined earlier (remember the '&')
        -1, // character limit, in case you want to use text typing
        "hello world!" // the string you want to print
      );
      ```
- after you've called that as many times as you need (up to 200 non-whitespace characters at once), render all of your printed text with `axotext_render`
  - like this:
    - ```c
      axotext_render();
      ```
    - you can change the character buffer size, `AXOTEXT_BUFFER_SIZE`, in `axotext.h` to allow for more characters to be rendered
    - note that higher buffer sizes will use more memory
- note that font size, line height, x position, and y position are actually floats. this engine allows for subpixel positioning at up to 4x precision, meaning the smallest unit for these is actually 0.25 pixels
- have fun :)
