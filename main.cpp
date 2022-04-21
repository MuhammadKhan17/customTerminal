#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
using namespace std;

vector<string> myImp();
int andChecker(vector<string> a);
int dollarChecker(vector<string> a);
//the code in the following 2 functions is credited to Alejandro Garcia
//https://d2l.ucalgary.ca/d2l/le/content/426489/viewContent/5188776/View
void dCommand1(vector<string> a, int dIndex);
void dCommand2(vector<string> a, int dIndex);
int main(){
    //this is to make sure that the fork doesn't run every time
    pid_t childID=-1;
    //to find locations of $ sings
    int dLocation;
    //to take in repeated inputs and make system calls
    while(1<2){

        //to hold the individual commands and inputs
        vector<string> cmd;
        //to enter into the system later
        string cmdLine="";

        //myInp just collects input from the user
        cout<<"please enter a command: ";
        cmd=myImp();
        
        //checking for the &
        if(andChecker(cmd)==1){
            //creates the fork for the background process
            childID=fork();
            //checks for error
            if(childID<0){
                perror("fork");
            }
            //resets the loop for the child process allowing the parent to wrok in the background 
            //and the child to take a new input
            if(childID==0){
                //child
                continue;
            }

        }
        //just tells the parent to wait until the new input for the child is processed
        //this was mainly done for my error checking and to show that the child is running the new process
        //the parent is in the background
        if(childID>0)
            wait(NULL);

        //to check for a $ and get its index
        dLocation = dollarChecker(cmd);
        if(dLocation>=0){
            switch (dLocation){
                case 1:
                    dCommand1(cmd,1);
                    continue;
                case 2:
                    dCommand2(cmd,2);
                    continue;
                default:
                    cout<<"improper use of the $ sign";
                    continue;

            }
        }
            

        //combines all the inputs beck together to make a system call
        for(int x=0;x<cmd.size();x++){
            cmdLine+=cmd.at(x)+" ";        
        }
        //conducting the system call and checking for errors
        if(system(cmdLine.c_str())==-1){
            perror("system call failed");
        }


        //if a child process was created, it would be done at this point as the new input would have been sent to the system
        if(childID==0){
            //resets so it won't fork again
            childID=-1;
            exit(0);
        }
    }

    return 0;
}

//collects the input and returns it in a string vector
vector<string> myImp(){
    vector<string> cmd;
    int i=0;
    do{
        string inp;
        cin>>inp;
        cmd.push_back(inp);
        if(cin.get()=='\n'){
            break;
        }
        i++;
    }while(1<2);
    return cmd;

}

//to check for the &
int andChecker(vector<string> a){
    for(int i=0;i<a.size();i++){
        if(a[i]=="&")
            return 1;
    }
    return 0;
}

//to check for the $
int dollarChecker(vector<string> a){
    for(int i=0;i<a.size();i++){
        
        if(a.at(i)=="$")
            return i;
    }
    return -1;
}

void dCommand2(vector<string> a, int dIndex){
    for(int i=dIndex+1;i<a.size();i++){
        int fds[2];
        pipe(fds);
        pid_t pid = fork();
        if (pid == -1){
            perror("forking\n");
            return;
        }
        //this is the first child and last 2 run
        if (pid == 0){
            //Redirect standard input to be the read end of the pipe
            dup2(fds[0], fileno(stdin));

            //Close undeeded pipes
            close(fds[0]);
            close(fds[1]);

            //Run cat
            //Cat takes in a single input, but none is provided in this execvp call so it will use stdin
            //However, stdin now points to the read end of the pipe so cat will use the contents of the pipe (which already have the outputs of pwd and ls by this point in time)
            char* right1[] = {(char*)a.at(i).c_str(), NULL};
            execvp(right1[0], right1);
            perror("Error executing command on the right of $");
            exit(EXIT_FAILURE);

        //this is the parent, it contains the next childs
        }else{
            pid_t pid2 = fork();
            //Error
            if (pid2 == -1){
                perror("Error forking");
                return;
            }

            //Child # 2 (This is the second child to run 2nd command on the left, putting output into pipe)
            if (pid2 == 0)
            {
                //Note we don't need to redirect output here because child#3 (whom runs first) already did it

                //Close undeeded pipes
                close(fds[0]);
                close(fds[1]);
                //Run "ls", output will automatically go to pipe due to Child#3's redirection
                char* left2[] = {(char*)a.at(1).c_str(), NULL};
                execvp(left2[0], left2);
                perror("Error executing 2nd command on left of $");
                exit(EXIT_FAILURE);
            //parent containing the 3rd child
            }else{
                 pid_t pid3 = fork();
                //Error
                if (pid3 == -1)
                    perror("Error forking");
                //Child # 3 (This is the first child to run - it will run "pwd")
                else if (pid3 == 0)
                {
                    //Redirect stdout to fds[WRITE_END] so that execvp automatically writes to the pipe
                    dup2(fds[1], fileno(stdout));
                    //Close uneeded fds
                    close(fds[0]);
                    close(fds[1]);
                    //Execvp command
                    char* left1[] = {(char*)a.at(0).c_str(), NULL};
                    execvp(left1[0], left1);
                    perror("Error executing command on left of $");
                    exit(EXIT_FAILURE);
                }
                    //Parent first to run
                else
                {
                    int status;
                    close(fds[0]);
                    close(fds[1]);
                    //Wait for child 3
                    waitpid(pid3, &status, 0);
                }
                int status;
                //Wait for child2
                waitpid(pid2, &status, 0);
            }
            int status;
            //Wait for child1
            waitpid(pid, &status, 0);

        }

    }
}

void dCommand1(vector<string> a, int dIndex){
    for(int i=dIndex+1;i<a.size();i++){
        int fds[2];
        pipe(fds);
        pid_t pid = fork();
        if (pid == -1){
            perror("forking\n");
            return;
        }
        //this is the first child and last 2 run
        if (pid == 0){
            //Redirect standard input to be the read end of the pipe
            dup2(fds[0], fileno(stdin));

            //Close undeeded pipes
            close(fds[0]);
            close(fds[1]);

            //Run cat
            //Cat takes in a single input, but none is provided in this execvp call so it will use stdin
            //However, stdin now points to the read end of the pipe so cat will use the contents of the pipe (which already have the outputs of pwd and ls by this point in time)
            char* right1[] = {(char*)a.at(i).c_str(), NULL};
            execvp(right1[0], right1);
            perror("Error executing command on the right of $");
            exit(EXIT_FAILURE);

        //this is the parent, it contains the next childs
        }else{
            pid_t pid2 = fork();
            //Error
            if (pid2 == -1){
                perror("Error forking");
                return;
            }

            //Child # 2 (This is the second child to run the command on the left, putting output into pipe)
            if (pid2 == 0)
            {
               //Redirect stdout to fds[WRITE_END] so that execvp automatically writes to the pipe
                dup2(fds[1], fileno(stdout));
                //Close uneeded fds
                close(fds[0]);
                close(fds[1]);
                //Execvp command
                char* left1[] = {(char*)a.at(0).c_str(), NULL};
                execvp(left1[0], left1);
                perror("Error executing command on left of $");
                exit(EXIT_FAILURE);
            //parent 
            }else{
                 
                int status;
                close(fds[0]);
                close(fds[1]);
                //Wait for child 2
                waitpid(pid2, &status, 0);
            }
            int status;
            //Wait for child1
            waitpid(pid2, &status, 0);
        }

    }
}