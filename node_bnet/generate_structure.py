import random

NUM_WORDS = 500

def GenerateLatentTemplate(path,num):
    out = file(path,'w')
    out.write("***\n")
    out.write("object:retail,person\n")
    out.write("retail:" + ",".join(["o"+str(x) for x in range(1,num+1)])+",NA\n")
    out.write(\
"person:person1,person2,NA\n \
command:retail,person\n \
retail_com:info,price,review,NA\n \
person_com:whois,wheremet,train,NA\n \
gender:male,female,NA\n \
***\n \
object->command\n \
object->retail\n \
object->person\n \
person->gender\n \
command->retail_com\n \
command->person_com\n \
***\n \
object:1,1\n \
command:1,0,0,1\n")
    
def GenerateSpeechKeywords(path,num):
    out = file(path,'w')
    out.write("***\n")
    numbers = range(0,NUM_WORDS)
    for i in range(1,num+1):
        wordNums = random.sample(numbers,  5) #sample 5 numbers from 0-999
        out.write("retail:o"+str(i)+"|"+",".join(["w"+str(n) for n in wordNums]) + '\n')
    
    
if __name__=="__main__":
    latentPath = "./latent_template_"
    speechPath = "./speech_keywords_"

    numObjects = [10,100,1000,10000,100000,1000000]
    #numObjects = [5,10]
    for num in numObjects:
        print("num="+str(num))
        GenerateLatentTemplate(latentPath+str(num)+".txt",num)
        GenerateSpeechKeywords(speechPath+str(num)+".txt",num)
    
