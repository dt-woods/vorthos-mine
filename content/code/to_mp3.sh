# to_mp3.sh
# Convert the pcm binary audio file to MP3 using ffmpeg.
ffmpeg -f f32le -ar 44100 -ac 1 -i output_audio.pcm output.mp3