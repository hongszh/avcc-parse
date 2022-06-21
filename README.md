## 说明

这个小程序是用来解析avcC格式的codec data，并且生成SPS，PPS数据，在Android中送给解码器的数据必须是AnnexB格式的数据。

AnnexB格式就是在一个NALU前面加上三个或者四个字节，这些字节的内容是`0 0 0 1`或者`0 0 1`。当我们读取一个`H264`流的时候，一旦遇到`0 0 0 1`或者`0 0 1`，我们就认为一个新的NALU开始了，因此，这些用来做分隔符的字节，一般也被称为start code，起始码。

avcC则采用了另外一种方式。那就是在NALU前面写上几个字节，这几个字节组成一个整数（大端字节序）这个整数表示了整个NALU的长度。在读取的时候，先把这个整数读出来，拿到这个NALU的长度，然后按照长度读取整个NALU。

## SPS & PPS

在AnnexB中，SPS和PPS被当做了普通的NALU进行处理，而在avcC中，SPS和PPS信息被当做了特殊的信息进行了处理。在采用avcC打包的H.264流之中，我们可以看到被称之为extradata的数据，这段数据定义了这个H.264流的基本属性数据，包含了SPS和PPS数据。

在gstreamer的pipeline中通过增加`-v`参数，就可以方便的看到codec_data的输出，对于mp4格式来说，看到的codec_data就是avcc格式，对于ts格式来说，看到的codec_data就是Annex-B格式。

比如：

```bash

下面两条命令分别都能从播放h264.mp4和h264.ts文件中获得codec_data：

# gst-launch-1.0 filesrc location=/sdcard/h264.mp4 ! qtdemux ! fakesink -v

/GstPipeline:pipeline0/GstFakeSink:fakesink0.GstPad:sink: caps = video/x-h264, stream-format=(string)avc, alignment=(string)au, level=(string)4, profile=(string)high, codec_data=(buffer)01640028ffe1001a67
640028acd940780227e584000003000400000300c83c60c65801000668ebe3cb22c0, width=(int)1920, height=(int)1080, framerate=(fraction)25/1, pixel-aspect-ratio=(fraction)1/1
```

```bash
# gst-play-1.0 /sdcard/h264.ts -v

/GstPlayBin:playbin/GstURIDecodeBin:uridecodebin0/GstDecodeBin:decodebin0/GstH264Parse:h264parse0.GstPad:src: caps = video/x-h264, stream-format=(string)avc, width=(int)1920, height=(int)1080, framerate=(
fraction)25/1, coded-picture-structure=(string)frame, chroma-format=(string)4:2:0, bit-depth-luma=(uint)8, bit-depth-chroma=(uint)8, parsed=(boolean)true, alignment=(string)au, profile=(string)high, level
=(string)4, codec_data=(buffer)01640028ffe1001a67640028acd940780227e584000003000400000300c83c60c65801000668ebe3cb22c0
```


## build

```bash
g++ -fdiagnostics-color=always -g pps-sps-parse.cpp -o pps-sps-parse
```
## 生成sps,pps文件

执行程序生成sps.bin，pps.bin
```bash
./pps-sps-parse
```

## 结果对比
<br>

> codec_data，在android里面简称csd，通过gst-play可以方便的获取一个mp4文件的codec_data，
> 然后将buffer写到csd.bin文件，dump_buffer()函数可以生成。
> `gst-play-1.0 /sdcard/h264.mp4 --videosink=fakesink -v`

```bash
$ hexdump -C csd.bin
00000000  01 64 00 28 ff e1 00 1a  67 64 00 28 ac d9 40 78  |.d.(....gd.(..@x|
00000010  02 27 e5 84 00 00 03 00  04 00 00 03 00 c8 3c 60  |.'............<`|
00000020  c6 58 01 00 06 68 eb e3  cb 22 c0                 |.X...h...".|
0000002b
```

```bash
$ hexdump -C sps.bin
00000000  00 00 00 01 67 64 00 28  ac d9 40 78 02 27 e5 84  |....gd.(..@x.'..|
00000010  00 00 03 00 04 00 00 03  00 c8 3c 60 c6 58        |..........<`.X|
0000001e
```

```bash
$ hexdump -C pps.bin
00000000  00 00 00 01 68 eb e3 cb  22 c0                    |....h...".|
0000000a
```

<br>

---

## 参考

[gstreamer中解析生成android的csd-0和csd-1](https://blog.csdn.net/hongszh/article/details/125340309?spm=1001.2014.3001.5501)

[FFmpeg对sps/pps的解析和格式转换](https://cloud.tencent.com/developer/article/1335918)

[码流格式:Annex-B, AVCC(H.264)与HVCC(H.265)](https://blog.csdn.net/yue_huang/article/details/75126155)





