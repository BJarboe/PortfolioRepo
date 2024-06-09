import os, sys
from moviepy.editor import AudioFileClip as afc

def convert(mp4):
    base, ext = os.path.splitext(mp4)
    mp3_file = base + '.mp3'
    audio_clip = afc(mp4)
    audio_clip.write_audiofile(mp3_file)
    audio_clip.close()

if __name__ == '__main__':
    print('toMP3 Main Called')
    from tkinter import Tk, filedialog
    Tk().withdraw()
    mp4 = filedialog.askopenfilename()
    print('Converted file: ', mp4)
    convert(mp4)