#include <iostream>
#include <vector>
#include <sstream> 
#include <unistd.h> // for fork(), execvp(), pipe()
#include <fcntl.h> // for open()
#include <sys/wait.h> // for waitpid()
using namespace std;

void executeCommand(vector <string> &words, bool background, int in_fd, int out_fd){
    char *args[words.size() + 1];
    for(int i = 0; i < words.size(); i++){
        args[i] = (char *) words[i].c_str();
    }
    args[words.size()] = NULL;
    /*
        cout << "Executing command: ";
        for (int i = 0; i < words.size(); i++) {
            cout << args[i] << " ";
        }
        cout << endl;
    */ // debug
    pid_t pid = fork();
    if(pid < 0){
        cerr << "Fork failed." << endl;
        exit(1);
    }
    else if(pid == 0){ // child process
        if(in_fd != 0){
            dup2(in_fd, STDIN_FILENO); // redirect stdin
            close(in_fd);
        }
        if(out_fd != 1){
            dup2(out_fd, STDOUT_FILENO); // redirect stdout
            close(out_fd);
        }
        if(execvp(args[0], args) == -1){
            cerr << "Failed to execute command." << endl;
            exit(1);
        } 
    }
    else{ // parent process
        if(!background){
            waitpid(pid, nullptr, 0);
        }
    }
}

int main(){
    signal(SIGCHLD, SIG_IGN); // prevent zombies
    while(true){
        cout << ">";
        string command;
        getline(cin, command);
        if(command.empty())
            continue;
        if(command == "exit")
            break;
        stringstream ss;
        ss << command;
        string aWord;
        vector <string> words[2];
        int mode = 0;
        int numberOfWords = 0;
        bool background = false;
        // cut inputs
        while(ss >> aWord){
            //cout << aWord << endl; // debug
            if(aWord == "&"){
                background = true;
                continue;
            }
            else if(aWord == ">" || aWord == "<" || aWord == "|"){
                mode = (int) aWord[0];
                numberOfWords++;
                continue;
            }
            words[numberOfWords].push_back(aWord);
        }

        // redirection
        if(mode == (int) '>'){
            int out_fd = open(words[1][0].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(out_fd < 0){
                cerr << "Failed to open file for writing." << endl;
                continue;
            }
            executeCommand(words[0], background, 0, out_fd);
            close(out_fd);
        }
        else if(mode == (int) '<'){
            int in_fd = open(words[1][0].c_str(), O_RDONLY);
            if(in_fd < 0){
                cerr << "Failed to open file for reading." << endl;
                continue;
            }
            executeCommand(words[0], background, in_fd, 1);
            close(in_fd);
        }

        // pipe
        else if(mode == (int) '|'){
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                cerr << "Pipe creation failed." << endl;
                continue;
            }
            executeCommand(words[0], false, 0, pipefd[1]);
            close(pipefd[1]);
            executeCommand(words[1], background, pipefd[0], 1);
            close(pipefd[0]);
        }
        else{
            executeCommand(words[0], background, 0, 1);
        }
    }
}