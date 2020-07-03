#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
#include <cstring>
#include <vector>
#include <cctype>
#include <locale>
#include <algorithm>

using namespace std;

//fun todo list!
//  maybe make an output argument!?!
//  make a better help!!?!?!?!?

inline bool exists (const std::string& name) { //stackoverflow!!!
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}


std::vector<std::string> //not stackoverflow im pretty sure
split(
        const std::string& input,
        const std::string& delims)
{
    std::vector<std::string> ret;
    for (size_t start = 0, pos; ; start = pos + 1) {
        pos = input.find_first_of(delims, start);
        std::string token = input.substr(start, pos - start);
        if (token.length() > 0)  // ignore empty tokens
            ret.push_back(token);
        if (pos == std::string::npos) break;
    }
    return ret;
}


using namespace std;
vector<string> amountOccured(string str, string needle){ //stackoverflow!!!
char *p;

    // split the string by spaces in a
    vector<string> a = split(str, " ");
    vector<string> occur;
    // search for pattern in a
    int c = 0;
    for (int i = 0; i < a.size(); i++){
        if (needle == a[i]) {
            occur.push_back(a[i+1]);
        }
    }
    return occur;
}


string exec(string command){ //stackoverflow!!!
    char buffer[256];
    string result = "";

    FILE * pipe = popen(command.c_str(), "r");

    if(!pipe){
        return "couldn't make pipe :(";
    }

    while(!feof(pipe)){
        if(fgets(buffer, 256, pipe) != NULL){
            result += buffer;
        }
    }

    pclose(pipe);
    return result;

}


string addOption(string name, char** args, int argcount, string help, string def = ""){ //does the cool option thingy
    if(argcount > 1){
        for(int i = 0; i < argcount; i++){
            if(args[i] == "--" + name){
                if(argcount-1 == i){
                    cout << '\n' << name << " argument help:\n" << help << '\n';
                    exit(0);
                }
                else{
                        string value = args[i+1];
                        return value;
                }
            }
        }
        return def;
    }
    return "";
}


int main(int argc, char** argv){
    //The args that are needed
    /* Arguments
    0 Input file
    1 Delete Residual files (0)
    2 Silence Threshold (-50db)
    3 Sound Speed (1)
    4 Silent Speed (5)
    5 The Silence Duration (2000)
    */

    if(argc == 1){
        cout << "Arguments\n file (default: none)\n deleteResidualFiles (default: 1)\n silenceThreshold (default 50, meaning -50db)\n soundSpeed (default: 1)\n silentSpeed (default: 5)\n silenceDuration (default: 2)\nsilence (default: 0)";
        cout << "\n\nUse --[argument name] to get more information about it";
        exit(0);
    }

    //options!!!!
    string filepath = addOption("file", argv, argc, "The file to work on (required)");
    string deleteResidual = addOption("deleteResidualFiles", argv, argc, "Remove all files other than the input file and the output file (not required)", "1");
    string silenceThreshold = addOption("silenceThreshold", argv, argc, "The threshold to trigger \"silence\" from 0 (all sound) to 100 (nothing) (50)", "50");
    string soundSpeed = addOption("soundSpeed", argv, argc, "The speed at which the parts of the video that has sound plays at (1x) (min: 0.5, max: 100)", "1");
    string silentSpeed = addOption("silentSpeed", argv, argc, "The speed at which the parts of the video that does not have sound plays at (5x) (min: 0.5, max: 100", "5");
    string silenceDuration = addOption("silenceDuration", argv, argc, "The duration of the silence (in seconds) to trigger the \"silence\" (2s)", "0.5");
    string silence = addOption("silent", argv, argc, "Suppress most messages that ffmpeg displays & most the messages outputted by this program (1 (silence) | 0 (normal) | debug)", "0");

    if(argc == 2){
        filepath = argv[1];
    }

    if(filepath == ""){
        cout << "\nThe argument 'file' is required\n";
        exit(0);
    }

    if(stoi(soundSpeed) > 100){
        soundSpeed = "100";
    }
    if(stof(soundSpeed) < 0.5){
        soundSpeed = "0.5";
    }

    if(stoi(silentSpeed) > 100){
        silentSpeed = "100";
    }
    if(stof(silentSpeed) < 0.5){
        silentSpeed = "0.5";
    }

    //need a filename to replace filepath when needed
    std::size_t found = filepath.find_last_of("/\\");
    string filename = filepath.substr(found+1);
    string filedir = filepath.substr(0, found+1);
    replace(filedir.begin(), filedir.end(), '\\', '/');
    string loglevel = (silence == "0" ? "" : " -loglevel warning ");
    if(silence == "debug"){
        loglevel = " -loglevel debug ";
    }
    exec("mkdir \"" + filedir +"temp\""); //1/2: Command that is not portable, but way more portable than the second...
    //the stuff that actually detects silence
    string out = exec("ffmpeg -i \"" + filepath + "\" -af silencedetect=n=-" + string(silenceThreshold) + "dB:d=" + silenceDuration + "  -f null - 2>&1");
    cout << out;
    vector<string> start = amountOccured(out.c_str(), "silence_start:");
    vector<string> send = amountOccured(out.c_str(), "silence_end:");
    for(int i = 0; i < start.size(); i++){
        start[i] = start[i].substr(0, start[i].find('[')-1);
    }
    string dur = exec("ffmpeg -i \"" + filepath + "\" -f null - 2>&1");
    vector<string>splited = split(amountOccured(dur.c_str(), "Duration:")[0], ":");
    float num = stoi(splited[0]) * 3600; //i could make this into one or two lines... but maybe later
    num += stoi(splited[1]) * 60;
    num += stof(splited[2]);
    start.push_back(to_string(num));

    vector<string> filenamesSound;
    vector<string> filenamesSilent;

    //for the part of the video where there ISN'T silence
    for(int i = 0; i < start.size(); i++){
         string command = "ffmpeg ";
         command.append(loglevel + " -to " + to_string(stof(start[i])) + " -ss "); //for some weird reason, it needs to be cast or else it will have 'frame=' on the last video clip
         if(i == 0){
            command.append("0");
         }
         else{
            command.append(send[i-1]);
         }
         command.append(" -y -i \""  + filepath + "\" -filter_complex \"[0:v]setpts=" + to_string(1/stof(soundSpeed)) + "*PTS[v];[0:a]atempo=" + soundSpeed + "[a]\" -map \"[v]\" -map \"[a]\" \"" + filedir +"temp/" + filename + to_string(i) + "sound.mp4\"" );
         if(start[i] != "0" && start[i] != send[abs(i)-1]){ //one of many error checking things that i have no idea why it works
            filenamesSound.push_back(filedir +"temp/" + filename + to_string(i) + "sound.mp4");
         }
         else{
            filenamesSound.push_back("0");
         }

         cout << '\n' << command << '\n';
         exec(command);
     }

     //for the part of the video where there IS silence

     for(int i = 0; i < start.size()-1; i++){
         string command = "ffmpeg ";
         command.append(loglevel + " -to " + send[i] + " -ss " + start[i]);
         command.append(" -y -i \""  + filepath + "\" -filter_complex \"[0:v]setpts=" + to_string(1/stof(silentSpeed)) + "*PTS[v];[0:a]atempo=" + silentSpeed + "[a]\" -map \"[v]\" -map \"[a]\" \"" + filedir +"temp/" + filename + to_string(i) + "silent.mp4\"" );
         filenamesSilent.push_back(filedir +"temp/" + filename + to_string(i) + "silent.mp4");
         cout << '\n' << command << '\n';
         exec(command);

     }

     //converts all files to .ts/mpeg-2, and it does not work with the stuff above, you can't convert and use a filtergraph at the same time
     for(int i = 0; i < filenamesSound.size(); i++){
        if(exists(filenamesSound[i])){
            exec("ffmpeg" + loglevel + " -y -i \"" + filenamesSound[i] + "\" -c copy -bsf:v h264_mp4toannexb -f mpegts \"" + filenamesSound[i] + ".ts\"");
        }
        else{
            filenamesSound[i] = "";
        }
     }
     for(int i = 0; i < filenamesSilent.size(); i++){
        if(exists(filenamesSilent[i])){
            exec("ffmpeg " + loglevel + "-y -i \"" + filenamesSilent[i] + "\" -c copy -bsf:v h264_mp4toannexb -f mpegts \"" + filenamesSilent[i] + ".ts\"");
        }
        else{
            filenamesSilent[i] = "";
        }
     }

    vector<string> correctOrder;
    //flip flops between starting with a sound clip or with a slient clip
     for(int i = 0; i < (filenamesSilent.size() > filenamesSound.size() ? filenamesSilent.size() : filenamesSound.size()) + 1; i++){
        if(filenamesSound[0] == "0"){ //this means the video starts with silence
            if(filenamesSilent.size() > i && filenamesSilent[i] != "" && exists(filenamesSilent[i] + ".ts")){
                correctOrder.push_back(filenamesSilent[i] + ".ts");
            }
            if(filenamesSound.size() > i+1 && filenamesSound[i] != "" && exists(filenamesSound[i] + ".ts")){
                correctOrder.push_back(filenamesSound[i+1] + ".ts");
            }
        }
        else{
            if(filenamesSound.size() > i && filenamesSound[i] != "" && exists(filenamesSound[i] + ".ts")){
                correctOrder.push_back(filenamesSound[i] + ".ts");
            }
            if(filenamesSilent.size() > i && filenamesSilent[i] != "" && exists(filenamesSilent[i] + ".ts")){
                correctOrder.push_back(filenamesSilent[i] + ".ts");
            }
        }
     }
     //i have to do this because of the command line character limit :(
     bool finishedConcat = false;
     int indexOrder = 0; //current index of the correctOrder vector to check if its done
     int numConcats = 0; //the number of concats to check if it needs to stitch it back to one file
     while(!finishedConcat){

        string command = "ffmpeg ";
        command.append(loglevel);
        command.append("-y -i \"concat:");
        cout << int(6000/(filedir.length()+filename.length()+8));
        for(int i = 0; i < int(1000/(filedir.length()+filename.length()+8)); i++){
            command.append(correctOrder[indexOrder] + "|"); //appends the filedirs to the command
            indexOrder++;
            if(indexOrder >= correctOrder.size()){
                i = 10000; //you can never be too sure ;)
                //break;
            }
        }
        command = command.substr(0,command.length()-1);
        command.append("\" \"" + filedir + "temp/finished_" + filename + to_string(numConcats) + ".ts\"");
        cout << command;
        exec(command); //concats!
        if(indexOrder >= correctOrder.size()){
            finishedConcat = true;
        }
        numConcats++;
     }

     string command = "ffmpeg ";
     command.append(loglevel);
     command.append("-y -i \"concat:");

     for(int i = 0; i < numConcats; i++){
        command.append(filedir + "temp/finished_" + filename + to_string(i) + ".ts|");
     }
     command = command.substr(0,command.length()-1);
     command.append("\" \"" + filedir + "finished_" + filename + "\"");
     cout << command;
     exec(command); //creates the final file!
     if(deleteResidual != "0"){
        exec("rmdir /Q /S \"" + filedir +"temp\""); //2/2: Command that is *not* portable between systems.
     }
     cout << "\n\nFinished, you can find your created file in the same directory as the input file you put. It is called: finished_" << filename;
     return 0;
}
