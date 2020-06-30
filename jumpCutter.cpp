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
//  make the silent mode - removes all ffmpeg logs
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


const char* addOption(string name, char** args, int argcount, string help, string def = ""){ //does the cool option thingy
    if(argcount > 1){
        for(int i = 0; i < argcount; i++){
            if(args[i] == "--" + name){
                if(argcount-1 == i){
                    cout << '\n' << name << " argument help:\n" << help << '\n';
                    exit(0);
                }
                else{
                    return args[i+1];
                }
            }
        }
        return def.c_str();
    }
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
        cout << "Arguments\n file\n deleteResidualFiles (0)\n silenceThreshold (50)\n soundSpeed (1)\n silentSpeed (5)\n silenceDuration (2s)";
        cout << "\n\nUse --[argument name] to get more information about it";
        exit(0);
    }

    //options!!!!
    string filepath = addOption("file", argv, argc, "The file to work on (required)");
    string deleteResidual = addOption("deleteResidualFiles", argv, argc, "Remove all files other than the input file and the output file (not required)", "0");
    string silenceThreshold = addOption("silenceThreshold", argv, argc, "The threshold to trigger \"silence\" from 0 (all sound) to 100 (nothing) (50)", "50");
    string soundSpeed = addOption("soundSpeed", argv, argc, "The speed at which the parts of the video that has sound plays at (1x)", "1");
    string silentSpeed = addOption("silentSpeed", argv, argc, "The speed at which the parts of the video that does not have sound plays at (5x)", "5");
    string silenceDuration = addOption("silenceDuration", argv, argc, "The duration of the silence (in seconds) to trigger the \"silence\" (2s)", "0.5");


    if(filepath == ""){
        cout << "\nThe argument 'file' is required\n";
        exit(0);
    }

    exec("mkdir temp"); //1/2: Command that is not portable, but way more portable than the second...

    //the stuff that actually detects silence
    string out = exec("ffmpeg -i " + filepath + " -af silencedetect=n=-" + string(silenceThreshold) + "dB:d=" + silenceDuration + "  -f null - 2>&1");
    cout << out;
    vector<string> start = amountOccured(out.c_str(), "silence_start:");
    vector<string> send = amountOccured(out.c_str(), "silence_end:");
    for(int i = 0; i < start.size(); i++){
        start[i] = start[i].substr(0, start[i].find('[')-1);
    }
    string dur = exec("ffmpeg -i " + filepath + " -f null - 2>&1");
    vector<string>splited = split(amountOccured(dur.c_str(), "Duration:")[0], ":");
    float num = stoi(splited[0]) * 3600; //i could make this into one or two lines... but maybe later
    num += stoi(splited[1]) * 60;
    num += stof(splited[2]);
    start.push_back(to_string(num));

    vector<string> filenamesSound;
    vector<string> filenamesSilent;

    //for the part of the video where there ISN'T silence

    for(int i = 0; i < start.size(); i++){
         string command = "ffmpeg -to ";
         command.append(to_string(stof(start[i]))); //for some weird reason, it needs to be cast or else it will have 'frame=' on the last video clip
         command.append(" -ss "); //don't ask why i have so many .append's, i just felt like it
         if(i == 0){
            command.append("0");
         }
         else{
            command.append(send[i-1]);
         }
         command.append(" -y -i "  + filepath + " -filter_complex \"[0:v]setpts=" + to_string(1/stof(soundSpeed)) + "*PTS[v];[0:a]atempo=" + soundSpeed + "[a]\" -map \"[v]\" -map \"[a]\" temp/" + filepath + to_string(i) + "sound.mp4" );
         if(start[i] != "0" && start[i] != send[i-1]){ //one of many error checking things that i have no idea why it works
            filenamesSound.push_back("temp/" + filepath + to_string(i) + "sound.mp4");
         }
         else{
            filenamesSound.push_back("0");
         }
         cout << '\n' << command << '\n';
         exec(command);

     }

     //for the part of the video where there IS silence

     for(int i = 0; i < start.size()-1; i++){
         string command = "ffmpeg -to ";
         command.append(send[i]);
         command.append(" -ss ");
         command.append(start[i]);
         command.append(" -y -i "  + filepath + " -filter_complex \"[0:v]setpts=" + to_string(1/stof(silentSpeed)) + "*PTS[v];[0:a]atempo=" + silentSpeed + "[a]\" -map \"[v]\" -map \"[a]\" temp/" + filepath + to_string(i) + "silent.mp4" );
         filenamesSilent.push_back("temp/" + filepath + to_string(i) + "silent.mp4");
         cout << '\n' << command << '\n';
         exec(command);

     }

     //converts all files to .ts/mpeg-2 (could've done this from the other one but, again, i was too lazy to do so.
     string command = "ffmpeg -y -i \"concat:";
     for(int i = 0; i < filenamesSound.size(); i++){
        if(exists(filenamesSound[i])){
            exec("ffmpeg -y -i " + filenamesSound[i] + " -c copy -bsf:v h264_mp4toannexb -f mpegts " + filenamesSound[i] + ".ts");
        }
        else{
            filenamesSound[i] = "";
        }
     }
     for(int i = 0; i < filenamesSilent.size(); i++){
        if(exists(filenamesSilent[i])){
            exec("ffmpeg -y -i " + filenamesSilent[i] + " -c copy -bsf:v h264_mp4toannexb -f mpegts " + filenamesSilent[i] + ".ts");
        }
        else{
            filenamesSilent[i] = "";
        }
     }


    //flip flops between starting with a sound clip or with a slient clip
     for(int i = 0; i < (filenamesSilent.size() > filenamesSound.size() ? filenamesSilent.size() : filenamesSound.size()) + 1; i++){
        if(filenamesSound[0] == "0"){ //this means the video starts with silence
            if(filenamesSilent.size() > i && filenamesSilent[i] != ""){
                command += filenamesSilent[i] + ".ts|";
            }
            if(filenamesSound.size() > i+1 && filenamesSound[i] != ""){
                command += filenamesSound[i+1] + ".ts|";
            }
        }
        else{
            if(filenamesSound.size() > i && filenamesSound[i] != ""){
                command += filenamesSound[i] + ".ts|";
            }
            if(filenamesSilent.size() > i && filenamesSilent[i] != ""){
                command += filenamesSilent[i] + ".ts|";
            }
        }
     }
     command = command.substr(0,command.length()-1);



     command.append("\" -c copy -bsf:a aac_adtstoasc finished_" + filepath);
     cout << command;
     exec(command); //creates the final file!
     if(deleteResidual != "0"){
        exec("rmdir /Q /S temp"); //2/2: Command that is *not* portable between systems.
     }
     return 0;
}
