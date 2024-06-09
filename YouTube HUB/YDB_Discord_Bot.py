import os, sys
from random import choice
from dotenv import load_dotenv
from discord import Intents, Client, Message, File
from pytube import YouTube


destination = str(os.path.dirname(__file__)) + '/vidCache'

# Handle Environment
load_dotenv()
TOKEN = os.getenv('DISCORD_TOKEN')

# Bot setup
intents = Intents.default()
intents.message_content = True      #NOQA
client = Client(intents=intents)


# Bot Startup
@client.event
async def on_ready() -> None:
    print(f'{client.user} is now running')


# Handling incoming messages
@client.event
async def on_message(message: Message) -> None:
    if message.author == client.user:
        return 
    
    username = str(message.author)
    user_message = message.content
    channel = str(message.channel)

    print(f'[{channel}] {username}: "{user_message}"')
    
    if client.user.mentioned_in(message):
        await message.channel.send(first_quip())
        link = parselink(message.content)
        print(f'Parsed link = {link}')
        
        if 'mp3' in message.content or 'audio' in message.content:
            mp3 = True
            print('**Audio Only Mode**')
        else:
            mp3 = False
        
        feedback = download_vid(link, mp3)

        if mp3 and feedback[len(feedback)-1] == '4':
            os.rename(feedback, feedback[:len(feedback)-1] + '3')
            feedback = feedback[:len(feedback)-1] + '3'
        if 'Exception' in feedback:
            if 'youtube' in link or 'youtu.be' in message.content:
                await message.channel.send(feedback)
                return
            else:
                await message.channel.send(second_quip())
        else:
            try:
                f = File(feedback)
                print('Created file')
                await message.channel.send(file=f)
                print('Success, deleting file from os')
                os.remove(feedback)
            except Exception as e:
                await message.channel.send(f'Problem downloading your video: {e}')
        

# Video Downloader
def download_vid(link: str, mp3: bool) -> str:
        print('Fetching Youtube Object..')
        try:
            yt = YouTube(link)
            print('Getting Stream Component..')
            yts = yt.streams.filter(only_audio=True).first() if mp3 else yt.streams.get_highest_resolution()
        except Exception as e:
                return f'Exception occured: invalid youtube link: <{link}>\n{e}'

        print('Downloading stream..')
        return yts.download(destination)


# Isolate Youtube link
def parselink(l: str) -> str:
    if 'youtube.com' or 'youtu.be' in l:
        start = -1
        for i in range(len(l)):
            if start == -1 and l[i:i+5] == 'https':
                start = i
            if start != -1 and l[i] == ' ':
                return l[start:i]
    return l[start:]


def first_quip() -> str:
    return choice([
        'Lemme cook..', 'ちょっと待って…',  'Hol up mein freund',
        '*Gong Sound*', 'Initializing ydb: a PyTube-equipped discord bot',
        'Interesting choice.. :face_with_raised_eyebrow:'
    ]) + '\n(Loading..)'

def second_quip() -> str:
    return choice([
        'Send a youtube video link numbnuts..', 'Epic bot usage fail\nLaugh at this user', 'Ahh HELL Nahh'
    ])

# Main entry point
def main() -> None:
    client.run(token=TOKEN)
    

if __name__ == '__main__':
    main()


