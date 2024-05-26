# pcc
pixel color converter<br/>
this is a commandline tool to convert any color to other color (even grayscale colors) while keeping the degree changing.<br/>
the tolerance check algorithm isn't perfect yet. which makes it hard to produce good videos. but when manually choosing recolored parts in pictures the color converting algorithm does the job well.<br/>
the cnv is the script that uses imagemagick to convert pictures to the supported image format (rgb 24bpp) and then reconvert the edited image to new image file named result.png.<br/>
cnvid is the script that uses ffmpeg to extract the frames to be recolored then reconvert them to new video file named video.mp4
