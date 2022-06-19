
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





