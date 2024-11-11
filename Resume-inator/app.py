import os, dotenv, pdfkit
from openai import OpenAI


#####################################################################
########################### Configuration ###########################
def check_config():
    dotenv.load_dotenv()
    try:
        with open('.env') as _:
            pass
        os.environ['OPENAI_API_KEY']
        os.environ['WK_PATH']
        os.environ['DIR_PATH']

    except:
        env_config()

def env_config():
    # create .env
    if not os.path.exists('.env'):
        with open('.env', 'w'):
            print('Created new .env file.\n')
            pass
    
    # Configure OpenAI API Key
    while not dotenv.get_key('.env', 'OPENAI_API_KEY'):
        api_key = input('Provide valid OPENAI API key:  ')
        try:
            dotenv.set_key('.env', 'OPENAI_API_KEY', api_key)
            dotenv.load_dotenv()
            OpenAI(api_key=os.environ['OPENAI_API_KEY']) # test api key
            

        except Exception as e:
            print(f'Invalid key provided:\n{e}\n')
            dotenv.unset_key('.env', 'OPENAI_API_KEY')
    print('1 / 3  API Key validated.\n')
    
    # Configure wkhtmltopdf path
    wk_path = r"resume-inator_venv\wk_dependencies\bin\wkhtmltopdf.exe"
    print('Setting wkhtmltopdf to default:  ', wk_path)

    invalid_path = True

    while invalid_path:
        try:
            pdfkit.configuration(wkhtmltopdf=wk_path) # test path to executable
            invalid_path = False
        except:
            wk_path = input(f'Invalid path to wkhtmltopdf.exe:  {wk_path}\nProvide path manually:  ')
    dotenv.set_key('.env', 'WK_PATH', wk_path)
    dotenv.load_dotenv()
    print('2 / 3  Path to wkhtmltopdf.exe set.\n')
    
    

    # Configure output path
    while not dotenv.get_key('.env', 'DIR_PATH'):
        directory_path = input('3.  Provide/Create directory for output\n(WATCH TYPOS):  ')

        try:
            if not os.path.exists(directory_path):
                os.makedirs(directory_path)
                print(f'Created {directory_path}')
            dotenv.set_key('.env', 'DIR_PATH', directory_path + '\\')
            dotenv.load_dotenv()
        except Exception as e:
            print(f'Something went wrong..\n{e}\n')
            
    print('3 / 3  Output Directory set.\n')




####################################################################
############################## Main ################################

def __main__():
    
    check_config()

    # Get env variables
    dotenv.load_dotenv()
    client = OpenAI(api_key=os.environ['OPENAI_API_KEY'])
    wk_config = pdfkit.configuration(wkhtmltopdf=os.environ['WK_PATH'])
    options = {'disable-smart-shrinking': None, 
               'page-size': 'A4', 
               'margin-top': '5mm' }
    dir_path = os.environ['DIR_PATH']


    # Set Model/Parameters
    model = 'gpt-4o-mini'
    temperature = 0.7
    max_tokens = 250 # unused by default


    # Assemble Chat prompt
    with open('input/prompt.txt', 'r') as f:
        prompt = f.read()

    while True:
        try:
            with open('input/resume.txt') as f:
                prompt += '\nMy Resume:\n' + f.read()
            break
        except:
            print('Couldn\'t find resume file.')
            print('Paste your resume information in input/resume.txt ..')
            if input('[q] to quit, [] to check for file..\n').lower() in {'q', 'quit'}: exit()

    with open('input/template.html', 'r') as f:
        prompt += '\nTemplate html:\n' + f.read()

    while True:
        try:
            with open('input/job_desc.txt', 'r') as f:
                prompt += '\nJob Description:\n' + f.read()
            break
        except:
            print('Couldn\'t find job description file.')
            print('Paste your job description in input/job_desc.txt ..')
            if input('[q] to quit, [] to check for file..\n').lower() in {'q', 'quit'}: exit()



    # Handle file naming
    file_name = input('Enter Resume Title:  ') + '_resume'
    html_file = dir_path + file_name + '.html'
    pdf_file = dir_path + file_name + '.pdf'

    # Generate and store response
    messages = []
    messages.append({"role":"user","content":prompt})
    try:
        print('Awaiting Response from OpenAI..')
        response = client.chat.completions.create(
            model=model,
            messages=messages,
            # max_tokens=max_tokens,  # uncomment for token limit
            temperature=temperature
        )
        content = response.choices[0].message.content
        with open(html_file, 'w') as f:
            f.write(content)
        
    except Exception as e:
        print(f'Response Error: {e}')

    print('Response Successful.  Converting to pdf..')
    
    # Convert response to pdf
    try:
        pdfkit.from_file(html_file, pdf_file, configuration=wk_config, options=options)
    except Exception as e:
        print('PDF conversion failure:\n', e)

    print(f'Created {pdf_file}\nFeel free to make any desired changes to the html file..')
    while input('Would you like to re-export any changes made?\n[no]  ').lower() in {'yes', 'y'}:
        try:
            pdfkit.from_file(html_file, pdf_file, configuration=wk_config, options=options)
            print('Successfully implemented changes!\n')
        except Exception as e:
            print('PDF conversion failure:\n', e)
    print('やったー！')

__main__()