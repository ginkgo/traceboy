# Image grid demo

This is a small demo page showing animated clips generated from GB trace files in a dynamic grid.

You need to generate the WebPs and `images.txt` file for this to work. To do this build `traceboy` and call the `make_webps.sh` script in the `traceboy` root path.

```
$ make -j8
$ scripts/make_webps.sh 
```

WebP generation will take a while since it needs to simulate every trace and encode an animated WebP image.

After that you can start a web server hosting the `image_grid` directory.
You can use whatever server you want. Using Python `http.server` works for instance:

```
$ cd image_grid/
$ python3 -m http.server 8080
```

You can then check out the hosted page on `http://0.0.0.0:8080/`.

