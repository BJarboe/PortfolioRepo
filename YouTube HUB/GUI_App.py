print('Initializing..')
from pytube import YouTube
from moviepy.editor import AudioFileClip as afc
import sys, os, traceback
import customtkinter as ctk
from PIL import Image

version = '0.2'
current_directory = os.path.dirname(__file__)
env = os.path.join(current_directory, '.yt-env')


def mp3_convert(mp4):
    base, ext = os.path.splitext(mp4)
    mp3_file = base + '.mp3'
    audio_clip = afc(mp4)
    audio_clip.write_audiofile(mp3_file)
    audio_clip.close()

def env_config(flag):
    if os.path.isfile(env):
        f = open(".yt-env", "r")
        tmp = f.read()
        f.close()
    print('Environment Configuration..')
    destination = ctk.filedialog.askdirectory(initialdir=current_directory, title="Choose Download Folder")
    if destination == "":
        return tmp
    f = open(".yt-env", flag)
    f.write(destination)
    f.close()
    return destination

def get_env():
    f = open(".yt-env", "r")
    destination = f.read()
    f.close()
    return destination

class MenuScreen(ctk.CTkFrame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        print('Entering Main Menu..')

        self.configure(width=200,height=600, fg_color='#242424')

        self.logo_init = ctk.CTkImage(
            light_image=Image.open('assets/yth_logo.png'),
            dark_image=Image.open('assets/yth_logo.png'),
            size=(150,150))
        self.logo = ctk.CTkLabel(self, text="", image=self.logo_init, width=400,height=200)
        self.logo.pack()

        self.logotxt = ctk.CTkLabel(self, text="Welcome to YT-HUB!", font=("Helvetica", 18))
        self.logotxt.pack(pady=10)
        
        self.VidDlButton = ctk.CTkButton(self, text="Download Videos", command=master.dlMenu)
        self.VidDlButton.pack(pady=10)

        self.settings_btn = ctk.CTkButton(self, text="Settings", command=master.settingsMenu)
        self.settings_btn.pack(pady=10)

        self.exit_btn = ctk.CTkButton(self, text="Exit", command=exit)
        self.exit_btn.pack(pady=40)

class DownloadScreen(ctk.CTkFrame):
    def __init__(self, master, **kwargs):
        super().__init__(master,**kwargs)
        print('Entering Downloader Menu..')
        self.configure(width=300,height=500, fg_color='#242424')

        self.instructions = ctk.CTkLabel(self, 
                                         text="Provide the link for the video you want to download", 
                                         padx=20, pady=20, corner_radius=20, 
                                          text_color='green', compound='left')
        self.instructions.pack(pady=10)

        self.link_entry = ctk.CTkEntry(self, width=300, height=40, corner_radius=20,
                                       placeholder_text="Paste Youtube link here")
        self.link_entry.pack(pady=10)

        self.mp3box = ctk.CTkCheckBox(self, text="Audio Only")
        self.mp3box.pack(pady=10)

        self.download_button = ctk.CTkButton(self, text="Download", command=self.download_vid)
        self.download_button.pack(pady=10)

        self.back_button = ctk.CTkButton(self, text="Back", command=master.mainMenu)
        self.back_button.pack(pady=10)

        

    def download_vid(self):
        print('Download initiated')
        self.instructions.configure(text="Download initiated, please wait..")
        link = self.link_entry.get()

        print('Checking environmental variable..')
        if not os.path.isfile(env):
            destination = env_config("x")
        else:
            destination = get_env()
            
        print('Fetching Youtube Object..')
        try:
            yt = YouTube(link)
            yts = yt.streams.get_highest_resolution()
        except Exception as e:
            print('Exception occured: invalid youtube link\n', e, '\nTraceback:\n', traceback.format_exc())
            self.link_entry.delete(0,len(link))
            self.instructions.configure(text=('Invalid link provided:\n' + link), text_color='red')
            return

        print('Downloading stream..')
        yts = yts.download(destination)

        if self.mp3box.get():
            print('Converting Audio..')
            mp3_convert(yts)
            os.remove(yts)

        self.instructions.configure(text=f'Finished downloading..\n{yt.title}', text_color='green')
        print(f'Successfully downloaded {yt.title}')


class SettingsScreen(ctk.CTkFrame):
    def __init__(self, master, **kwargs):
        super().__init__(master, **kwargs)
        print("Entering Settings Menu..")
        self.configure(width=300,height=500, fg_color='#242424')
        
        env_txt = self.env_text()

        self.prompt = ctk.CTkLabel(self, text=env_txt, padx=20, pady=20, corner_radius=20, text_color='green', compound='left')
        self.prompt.pack(pady=10)

        self.dlpath_btn = ctk.CTkButton(self, text="Configure Download Path", command=self.env_reconfig)
        self.dlpath_btn.pack(pady=10)

        self.back_btn = ctk.CTkButton(self, text="Back", command=master.mainMenu)
        self.back_btn.pack(pady=10)

    def env_reconfig(self):
        if not os.path.isfile(env):
            print('Setting new env')
            destination = env_config("x")
        else:
            print('Overwriting env')
            destination = env_config("w")
        print(os.path.isfile(env))
        print('Current path: ', self.env_text())
        self.prompt.configure(text=self.env_text())

    def env_text(self):
        if os.path.isfile(env):
            return f'Current download path:\n{get_env()}'
        else:
            return 'No current download path'



class App(ctk.CTk):
    def __init__(self):
        super().__init__()
        print("Initializing YT-Hub..")
        self.title("Youtube Hub v" + version)
        self.geometry("480x640")
        self.mainMenu()

    def dlMenu(self):
        self.clear()
        self.menu = DownloadScreen(master=self)
        self.menu.pack(expand=False, pady=80)
    
    def mainMenu(self):
        self.clear()
        self.menu = MenuScreen(master=self)
        self.menu.pack(expand=True, pady=(80,100))

    def settingsMenu(self):
        self.clear()
        self.menu = SettingsScreen(master=self)
        self.menu.pack(expand=True, pady=(80,300))

    def quit_callback(self):
        print('Exitting..')
        app.destroy()

    def clear(self):
        for widget in self.winfo_children():
            widget.destroy()

if __name__ == '__main__':
    app = App()
    app.mainloop()