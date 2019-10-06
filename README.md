# 🍔 slider
slider is a tiny tool for creating slideshows.

See [demo.txt]() for an example of a slidehow and [demo.pdf]() for the output. This slideshow also serves as a quick introduction to slider.

## usage
    slider [options] file [output file]

    options:
      -d	--dark	dark mode
      -4	--4:3   render pages in 4:3 aspect ratio instead of 16:9 (the default)
      -h	--help	show this message

    format codes:
      !	monospace
      <	left align
      -	small font

The input is a text file. One line = one slide. Here's an example:

    this is a slide
    this is also a slide

Long lines of text are wrapped, or you can manually wrap them by using `\n`:

    this long line of text will be wrapped so it doesn't overflow
    you can also\nmanually wrap lines\nlike this

Start a line with a format code to change the look of the slide:

    !this slide will be monospace
    <this slide will be left aligned
    -this slide will use a smaller font

You can also combine them, all three together works well for code:

    !<-this will be monospace, left aligned and use a smaller font

## building
slider builds and runs under Windows and Linux. OSX should work too, I just haven't tried it.

The visual studio solution is included. Use [premake 5](https://premake.github.io/) to generate other project files.