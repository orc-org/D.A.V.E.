#include <cstdio>
#include <string>
#include <array>
#include <chrono>


using namespace std;
using namespace std::chrono_literals;

struct TrieNode{ // node used for Binary  Tree (named funny so as to not clash with rclcpp::node)
    // Attributes
    string letter;
    TrieNode *leftChild;
    TrieNode *rightChild;

    // Constructors
    TrieNode() {
        letter = " ";
    }

    TrieNode(string aLetter, TrieNode *aLeftChild, TrieNode *aRightChild){
        letter = aLetter;
        leftChild = aLeftChild;
        rightChild = aRightChild;
    }
};

//////////////////////////////////////////////////////////////////////////
// Read me (as seen in readme.md)
/////////////////////////////////////////////////////////////////////////

/*

Author: Isaac Meyer, isaac.bobmeyer@gmail.com
Date: March 4, 2026
Purpose: to fullfill certain task specific requirements for the ORC club to compete at CIRC 2026

Description:

    this ROS node will take in a video signal and detect a flashing light
    this flashing light will be read as morse code and translated into english 
    (this is how we extract the password for the linux terminal in the 2026 heist mission)
    
    this ROS node will also allow the user to input a string that they wish to have translated to morse code
    this message will then be translated into a signal that will actuate some sort of appendage that will enter the message on a physical morse key
    (this is how we"ll input the vault password for the 2026 heist mission)

    for our Morse code encoding/decoding (string representation):
    dits are "*",
    daws are "-" 
    and spaces " " will be inserted inbetween them to differentiate between letters
    multiple spaces will indicate the spaces in between words (if we encounter that)s

    For the actual encoding/decoding:
     -> an array containing the morse code representations for each character that will be used for 
        translating to morse code. this will work like a static hash table, we can calculate the index of the 
        morse representation of any character from it's ascii value
        this method boasts O(1) time complexity for look up and O(n) space complexity while still being damn simple to implement

     -> a Binary Trie will be used for translating from morse code to english,
        starting from the root node, if the next character in the morse code message is a dit, we traverse
        to the left child, it it's a daw, we traverse to the right child and if the next character is a space
        then we've found the correct character and can read it from the node.
        this method boasts O(log(n)) time complexity for look up and O(n) space complexity
        this is a more advanced implementation that an array with linear search to find the correct character,
        but it is much efficient (all for the low price of storing this data twice in memory).
        with the computing power of D.A.V.E, this probably doesn't matter much, but I can make it better, so I will

    any time "morse table" is refered to, I'm talking about the array that holds the data for what combination of dits and daws make which letter
    any time "morse key" is refered to, I'm talking about the phisical button that the rover will need to tap to enter the password to the vault door
    
    "TrieNode" is used in the context of the Trie. it is named like that to avoid getting confused with rclcpp::Node

    "the appendage" referes to whatever physical part of the rover gets actuated to hit the morse key, at the time of writting this, the hand design
    has yet to be finalized, hence the non-descriptive term

    This ROS node comes built in with two testing methods (named "testingProtocol1" and "testingProtocol2")

    Description of/How to use  the testing Protocols:
       
        TESTING PROTOCOL 1:
            I will make 2 practice morse keys that will illuminate an LED, one for the tester and one for D.A.V.E to use
       
            to test, first call the toggleTestMode() method to enable test mode (disabled by default)

            the tester can input some message onto their morse key, 
            D.A.V.E will then detect the flashing LED, decipher the message and then reply with a specified response phrase
       
            since we're not simply having the rover repeat what we said to it, but reply with something else
            we're testing how well it interperets morse, not just its ability to replicate it

            example tests:
                Tester:  "HelloThere"
                D.A.V.E: "GeneralKenobi"
            
                Tester:  "apple"
                D.A.V.E: "fruit"

                Tester:  "WhatYou"
                D.A.V.E: "egg"

            the only downside to this testing method is that the tester must be able to use morse code (or atleast decifer it given enough time)

        TESTING PROTOCOL 2: 
            testing protocol just makes it so that the rover will enter what it reads to the morse key as soon as it's done encoding/decoding the meassage
            
            this will essentially test if data can be read from the camera, decoded and then re-encoded without any errors

            going about it this way will allow us to test the 3 decoding cases

            much like in testing protocol 1, we'll use 2 practice morse keys to communicate with the rover

        the biggest concern of mine is the ability to decode the morse signal from the camera, hence why both testing protocols are aimed at verifying
        this funciton

        to test the encoding, simply set the vault password manually and call the sentToAppendage() method and see what it taps out.
        odds are, if I've nailed the decoding, then the encoding should also be good, 
        but i"ll leave it up to you to figure out exactly how right I am

        note: testing protocol 1 trumps testing protocol 2, so if you want to run testing protocol 2, be sure to deactivate testing protocol 1

*/


class EnigmaMachine {
//////////////////////////////////////////////////////////////////////////
// Attributes
/////////////////////////////////////////////////////////////////////////

string letters[39];      // array to hold the key to translate between morse code and english
TrieNode *treeRoot;          // root node of the binary tree used to translate from morse code to english 

string message;          // string to hold the message to be encoded and sent to the appendage used to articulate morse key
string morseMessage;     // string to hold the translated message to be sent to the appendage used to articulate morse key
string terminalPassword; // string to store the password to the linux terminal

bool activateTestingProtocol1 = false; // boolean to deactivate test case 1 
bool activateTestingProtocol2 = false; // boolean to deactivate test case 2
// (since if by some weird cooincidence, the password is one of our test phrases, we don't want the rover to go into the testing protocol)

int ditPulse;          // to store the pulse width of a dit
int dawPulse;          // to store the pulse width of a daw
int restTime;          // to store how long inbetween dits and daws (probably will set equal to dit pulse length)
int letterSpacelength; // to store how long inbetween characters (this is definetly a thing, other wise "he" is indestinguishable from "5")
                         // I may decide to set the letter space length equal to the length of a daw, we'll see
int largeSpaceLength;  // to store how long in between words (not sure if this is a thing)

int decodeCase = 1;    // to allow us to switch between decoding methods (just incase there's a formatting issue with the terminal password)
int encodeCase = 1;    // to allow us to switch between encoding methods (just incase there's a formatting issue with the vault password)


//////////////////////////////////////////////////////////////////////////
// Getters
/////////////////////////////////////////////////////////////////////////

string getPassword(){
    return terminalPassword;
}

string getMessage(){
    return message;
}

string getMorseMessage(){
    return morseMessage;
}

//////////////////////////////////////////////////////////////////////////
// Setters
/////////////////////////////////////////////////////////////////////////

void setMessage(string message){
    message = message;
}

void setMorseMessage(string message){
    morseMessage = message;
}

//////////////////////////////////////////////////////////////////////////
// Other Functions
/////////////////////////////////////////////////////////////////////////

void testingProtocol1(string message);

string decode(string message){
    //look for where the word "stop" appears in the message and work back from there
    string stop = "*** - --- *--*";
    string decodedMessage = "";
    TrieNode *current = treeRoot;

    string firstWord = "";
    bool firstWordFound = false;

    if (activateTestingProtocol1){
        testingProtocol1(message);
    }
    else {

        switch (decodeCase){
        
        //implementation for if we assume a 1 word password ("stop" to end password)
        case 1:

            // set up start and end indices so we are looking inbetween two occurances of the word "stop"
            int start = message.find(stop) + stop.length() + 1; // +1 to account for space after stop
            int end = message.find(stop, start);
        
            for (int  i = start; i < end; i++){
                char letter = message.at(i);

                switch(letter){
                    // dit
                    case '*':
                        current = current->leftChild;
                        break;
                    // daw
                    case '-':
                        current = current->rightChild;
                        break;
                    // end of letter
                    case ' ':
                        decodedMessage.append(current->letter);
                        current = treeRoot;
                        break;
                }
            }
            break;

            //implementation for a multi word password with "stop" to denote the end of a password (will also work for single word password with "stop")
        case 2: 

            for (int j = 0; j < message.length(); j++){

                // set up start and end indices so we are looking inbetween two occurances of the word "stop"
                int start = message.find(stop) + stop.length() + 1; // +1 to account for space after stop
                int end = message.find(stop, start);
                
                //find next word in password
                string nextWord = "";
                for (int  i = start; i < end; i++){
                    char letter = message.at(i);

                    switch(letter){
                        // dit
                        case '*':
                            current = current->leftChild;
                            break;

                        // daw
                        case '-':
                            current = current->rightChild;
                            break;

                        // end of letter/word
                        case ' ':
                            nextWord.append(current->letter);
                            current = treeRoot;
                            break;
                    }
                }

                // find first word of password
                if (!firstWordFound){
                    firstWordFound == true;
                    firstWord = nextWord;
                    decodedMessage.append(firstWord);
                } 
                // if the password is not repeating itself, append the next word
                else if (firstWord != nextWord){
                    decodedMessage.append(" ");
                    decodedMessage.append(nextWord);
                }
                // if the password is repeating itself, then we've already found it so we break out of the while loop
                else {
                    break;
                }

            }
            break;

        
        
        //implementation for a multi word password without "stop" (will also work for single word password without "stop")
        case 3:
            int start = message.find("  ") + 1;  // since multiple spaces will be denote separate word (+1 to account for indexing of the second space)
            int end = message.find("  ", start); // find the end of the next word
            
            // we're using a for loop here to avoid an infinite loop in the case that we misread the password
            // for example, if the first word was stored as _thing instead of thing, this might not detect when the password repeats
            // if its searching for a space that is never recorded
            for (int j = 0; j < message.length(); j++){
                //find next word in password
                string nextWord = "";
                for (int  i = start; i < end; i++){
                    char letter = message.at(i);

                    switch(letter){
                        // dit
                        case '*':
                            current = current->leftChild;
                            break;

                        // daw
                        case '-':
                            current = current->rightChild;
                            break;

                        // end of letter/word
                        case ' ':
                            nextWord.append(current->letter);
                            current = treeRoot;
                            break;
                    }
                }

                // find first word of password
                if (!firstWordFound){
                    firstWordFound == true;
                    firstWord = nextWord;
                    decodedMessage.append(firstWord);
                } 
                // if the password is not repeating itself, append the next word
                else if (firstWord != nextWord){
                    decodedMessage.append(" ");
                    decodedMessage.append(nextWord);
                }
                // if the password is repeating itself, then we've already found it so we break out of the while loop
                else {
                    break;
                }

            }
            break;
        }
    }
    current = 0;

    if (activateTestingProtocol2){
        encode (decodedMessage);
        sendToAppendage();
    }
}


string encode(string message){
    //translatet message into morse code
    string stop = "*** - --- *--*";
    string encodedMessage = "";
    
    
    //implementation for if we assume a 1 line password ("stop" to end password)
    for (unsigned int i = 0; i < message.length(); i++){
        int letter = message.at(i); // store ASCII value of charater at index i of the message
        int testCase = (letter > 96 && letter < 123) ? 0 : // it's a letter
                        (letter > 47 && letter < 58) ? 1 :         // it's a number
                        (letter == 46) ? 2:                        // it's a period
                        (letter == 44) ? 3:                        // it's a comma
                        (letter == 63) ? 4:                        // it's a question mark
                        (letter == 32) ? 5: -1;                    // it's a space

        switch (testCase){
            case 0: //letter
                encodedMessage.append(letters[i - 97]);
                break;
            case 1: //number
                encodedMessage.append(letters[i - 22]);
                break;
            case 2: //period
                encodedMessage.append(letters[36]);
                break;
            case 3: //comma
                encodedMessage.append(letters[37]);
                break;
            case 4: //Question mark
                encodedMessage.append(letters[38]);
                break;
            case 5: //space
                encodedMessage.append(" ");
                break;
        }
    }


    // this does not need to be a switch, it could be an if statement
    // I left it as a switch block so that if the formatting of the password at CIRC is different than we expected, we can easily add another case or 2
    // if additional cases (more than 3) must be added, be sure to increase the range of the g_encodeCase variable
    switch(encodeCase){
        // "stop" at the end of the password
        case 1:
            encodedMessage.append(stop);
            break;
        
        // no "stop" at the end of the password
        case 2:
            //add code here if it is needed
            break;

        //blank case for if CIRC throws us a curve ball
        case 3: 
            //add code if needed
            break;
    }
    
}

void sendToAppendage(){
    // send the translated message to the appendage

}

void testingProtocol1(string message){
    /* this method will be used when testing the capabilities of this node to accurately decode morse code
       I will make 2 practice morse keys that will illuminate an LED, one for the tester and one for D.A.V.E to use
       
       to test, first call the toggleTestMode() method to enable test mode (disabled by default)

       the tester can input some message onto their morse key, 
       D.A.V.E will then detect the flashing LED, decipher the message and then reply with a specified response phrase
       
       since we're not simply having the rover repeat what we said to it, but reply with something else
       we're testing how well it interperets morse, not just its ability to replicate it

       example tests:
            Tester:  "HelloThere"
            D.A.V.E: "GeneralKenobi"
            
            Tester:  "apple"
            D.A.V.E: "fruit"

            Tester:  "WhatYou"
            D.A.V.E: "egg"

            Tester:  "1468"
            D.A.V.E: "8641"

        the only downside to this testing method is that the tester must be able to use morse code (or atleast decifer it given enough time)
     */

    // generate an integer based on which message is received to allow us to switch on strings
    int testCase = (message == "Hello There") ? 0 : 
                    (message == "apple") ? 1 :
                    (message == "WhatYou") ? 2 :
                    (message == "1468") ? 3 : -1;

    // select appropriate response based on the message received
    switch (testCase){
        case 0:
            setMessage("General Kenobi");
            break;
        case 1:
            setMessage("fruit");
            break;
        case 2:
            setMessage("egg");
            break;
        case 3:
            setMessage("8641");
            break;
        default:
            setMessage("pardon?");
    }

    encode(getMessage());
    sendToAppendage();    
}

void setupMorseTable();

void toggleTestMode1(){
    activateTestingProtocol1 = !activateTestingProtocol1;
}

void toggleTestMode2(){
    activateTestingProtocol2 = !activateTestingProtocol2;
}

// 3 separate decode cases
void toggleDecodeCase(){
    decodeCase++;
    if (decodeCase == 4)
        decodeCase == 1;
}

// 3 separate encode cases 
void toggleEncodeCase(){
    encodeCase++;
    if (encodeCase == 4)
        encodeCase == 1;
}

void setupMorseTable(){
    // setup for translating from letter to morse code
    letters[0]  = "*-";     //a
    letters[1]  = "-***";   //b
    letters[2]  = "-*-*";   //c
    letters[3]  = "-**";    //d
    letters[4]  = "*";      //e
    letters[5]  = "**-*";   //f
    letters[6]  = "--*";    //g
    letters[7]  = "****";   //h
    letters[8]  = "**";     //i
    letters[9]  = "*---";   //j
    letters[10] = "-*-";    //k
    letters[11] = "*-**";   //l
    letters[12] = "--";     //m
    letters[13] = "-*";     //n
    letters[14] = "---";    //o
    letters[15] = "*--*";   //p
    letters[16] = "--*-";   //q
    letters[17] = "*-*";    //r
    letters[18] = "***";    //s
    letters[19] = "-";      //t
    letters[20] = "**-";    //u
    letters[21] = "***-";   //v
    letters[22] = "*--";    //w
    letters[23] = "-**-";   //x
    letters[24] = "-*--";   //y
    letters[25] = "--**";   //z
    letters[26] = "-----";  //0
    letters[27] = "*----";  //1
    letters[28] = "**---";  //2
    letters[29] = "***--";  //3
    letters[30] = "****-";  //4
    letters[31] = "*****";  //5
    letters[32] = "-****";  //6
    letters[33] = "--***";  //7
    letters[34] = "---**";  //8
    letters[35] = "----*";  //9
    letters[36] = "*-*-*-"; //period
    letters[37] = "--**--"; //comma
    letters[38] = "**--**"; //question mark

    // setup binary tree for translating from morse code to letter
    // '$' represents "void nodes" (nodes that do not correspond to a character, but are a stepping stone to nodes that do)
    // I realize this is some of the least readable spaghetti code ever written
    // but it should work, and i'll upload a tree diagram to git along with this ROS node
    treeRoot = new TrieNode(" ",
        // set up left tree (starts with dit)    
        new TrieNode("e", 
                new TrieNode("i", 
                    new TrieNode("s", 
                        new TrieNode("h", 
                            new TrieNode("5", nullptr, nullptr), 
                            new TrieNode("4", nullptr, nullptr)
                        ), 
                        new TrieNode("v", 
                            nullptr, 
                            new TrieNode("3", nullptr, nullptr)
                        )
                    ), 
                    new TrieNode("u", 
                        new TrieNode("f", nullptr, nullptr), 
                        new TrieNode("$", 
                            new TrieNode("$", 
                                new TrieNode("?", nullptr, nullptr), 
                                nullptr), 
                            new TrieNode("2", nullptr, nullptr)
                        )
                    )
            ), 
            new TrieNode("a", 
                new TrieNode("r", 
                    new TrieNode("l", nullptr, nullptr), 
                    new TrieNode("$",
                        new TrieNode("$", 
                            nullptr, 
                            new TrieNode(".", nullptr, nullptr)),
                        nullptr)
                ), 
                new TrieNode("w", 
                    new TrieNode("p", nullptr, nullptr), 
                    new TrieNode("j", 
                        nullptr, 
                        new TrieNode("1", nullptr, nullptr)
                    )
                )
            )
        ), 

        // setup right side of tree (starts with daw)
        new TrieNode("t", 
            new TrieNode("n", 
                new TrieNode("d", 
                    new TrieNode("b", 
                        new TrieNode("6", nullptr, nullptr), 
                        nullptr
                    ), 
                    new TrieNode("x", nullptr, nullptr)
                ), 
                new TrieNode("k", 
                    new TrieNode("c", nullptr, nullptr), 
                    new TrieNode("y", nullptr, nullptr)
                )
            ), 
            new TrieNode("m", 
                new TrieNode("g", 
                    new TrieNode("z", 
                        new TrieNode("7", nullptr, nullptr), 
                        new TrieNode("$", 
                            nullptr, 
                            new TrieNode(",", nullptr, nullptr))), 
                    new TrieNode("q", nullptr, nullptr) 
                ), 
                new TrieNode("o", 
                    new TrieNode("$",
                        new TrieNode("8", nullptr, nullptr),
                        nullptr
                    ),
                    new TrieNode("$", 
                        new TrieNode("9", nullptr, nullptr),
                        new TrieNode("0", nullptr, nullptr)
                    )
                )
            )
        )
    );

}

};


//////////////////////////////////////////////////////////////////////////
// TO-DO
/////////////////////////////////////////////////////////////////////////

/*
    - set up multiple cases for decoding and encoding (no stop word used, multi-line passwords, etc)

    - upload tree diagram for binary tree to better explain what's happening

    - verify that binary tree implementation will not cause a memory leak 
    (check if deleating an object that contains pointers will deleate those other pointers (i.e. can i deleate a tree by uprooting it))

    - make a function to force the message to be encoded to be all lower case (put at start of encode())

    - add methods to manipulate the decode/encode cases

    - fix binary tree (see comments below)
*/

//////////////////////////////////////////////////////////////////////////
// END TO-DO
/////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv)
{
  (void) argc;
  (void) argv;


  return 0;
}