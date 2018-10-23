This is png++ the C++ wrapper for libpng.  Version 0.2

General
=======

PNG++ aims to provide simple yet powerful C++ interface to libpng, the
PNG reference implementation library.

PNG++ is free software distributed under a modified variant of BSD
license.  For more details please refer to file COPYING in this
directory.

Using raw libpng in C++ may impose serious challenge since lots of
precautions must be taken to handle initialization/deinitialization of
control structures as well as handling errors correctly.  With png++
you can read or write PNG images just in a single line of C++ code:

png::image< png::rgb_pixel > image("input.png");
image.write("output.png");

The code reads an image from the file named "input.png", then writes
the image to a file named "output.png".  In this example png++ does
all the transformations needed to create adequate in-memory RGB
representation of the image (well, in most cases it simply instructs
libpng to do so).

The image in "input.png" can be RGB image, or it might be grayscale or
even indexed image with a palette--png++ will just convert any input
to RGB format.  However, for technical reasons such automatic
transformation is supported for RGB and Grayscale color types only.
Optionally there may be an alpha channel in the target color space
(RGBA and Gray+Alpha respectively).


Download
========

The project is hosted at Savannah:

	http://savannah.nongnu.org/projects/pngpp/

Released source packages can be found here:

	http://download.savannah.nongnu.org/releases/pngpp/

Also, you can check out sources directly from SVN repository:

	svn://svn.sv.nongnu.org/pngpp/trunk/

or, for people w/o outgoing svn:

	http://svn.sv.nongnu.org/pngpp/trunk/

Online documentation can be found here:

	http://www.nongnu.org/pngpp/doc/html/index.html


Help
====

There is a mailing list for developers:

	http://lists.nongnu.org/mailman/listinfo/pngpp-devel

You can also contact me by dropping a mail to <alex.shulgin@gmail.com>.


Happy hacking!
--
Alex Shulgin  <alex.shulgin@gmail.com>
