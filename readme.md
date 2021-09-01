# noaa-img-collector

Collect GOES16 infrared satalite images from the noaa website.

## Usage
Start collection yesterday
``` shell
$ ./noaa-img-collector $(date --date='yesterday' +%s)
```
Start collection now
``` shell
$ ./noaa-img-collector $(date +%s)
```
Start collection at this unixtime
``` shell
$ ./noaa-img-collector 1630426439
```
