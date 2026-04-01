Author: Isaac Meyer, isaac.bobmeyer@gmail.com

Date: March 4, 2026

Purpose: to fullfill certain task specific requirements for the ORC club to compete at CIRC 2026

# Description:

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

## For the actual encoding/decoding:

* an array containing the morse code representations for each character that will be used for 
  translating to morse code. this will work like a static hash table, we can calculate the index of the 
  morse representation of any character from it's ascii value
  this method boasts O(1) time complexity for look up and O(n) space complexity while still being damn simple to implement

* a Binary Trie will be used for translating from morse code to english,
  starting from the root node, if the next character in the morse code message is a dit, we traverse
  to the left child, it it's a daw, we traverse to the right child and if the next character is a space
  then we've found the correct character and can read it from the node.
  this method boasts O(log(n)) time complexity for look up and O(n) space complexity
  this is a more advanced implementation that an array with linear search to find the correct character,
  but it is much efficient (all for the low price of storing this data twice in memory).
  with the computing power of D.A.V.E, this probably doesn't matter much, but I can make it better, so I will

* any time "morse table" is refered to, I'm talking about the array that holds the data for what combination of dits and daws make which letter
* any time "morse key" is refered to, I'm talking about the phisical button that the rover will need to tap to enter the password to the vault door  
* "BTNode" is used in both the context of the binary tree. it is named like that to avoid getting confused with rclcpp::Node
* "the appendage" referes to whatever physical part of the rover gets actuated to hit the morse key, at the time of writting this, the hand design
has yet to be finalized, hence the non-descriptive term

This ROS node comes built in with two testing methods (named "testingProtocol1" and "testingProtocol2")

## Description of/How to use  the testing Protocols:
       
### TESTING PROTOCOL 1:
I will make 2 practice morse keys that will illuminate an LED, one for the tester and one for D.A.V.E to use
       
to test, first call the toggleTestMode() method to enable test mode (disabled by default)

the tester can input some message onto their morse key, 
D.A.V.E will then detect the flashing LED, decipher the message and then reply with a specified response phrase
       
since we're not simply having the rover repeat what we said to it, but reply with something else
we're testing how well it interperets morse, not just its ability to replicate it

example tests:
``
    Tester:  "HelloThere"
    D.A.V.E: "GeneralKenobi"
            
    Tester:  "apple"
    D.A.V.E: "fruit"

    Tester:  "WhatYou"
    D.A.V.E: "egg" ``

the only downside to this testing method is that the tester must be able to use morse code (or atleast decifer it given enough time)

### TESTING PROTOCOL 2: 
testing protocol just makes it so that the rover will enter what it reads to the morse key as soon as it's done encoding/decoding the meassage
            
this will essentially test if data can be read from the camera, decoded and then re-encoded without any errors

going about it this way will allow us to test the 3 decoding cases

much like in testing protocol 1, we'll use 2 practice morse keys to communicate with the rover

the biggest concern of mine is the ability to decode the morse signal from the camera, hence why both testing protocols are aimed at verifying this funciton

to test the encoding, simply set the vault password manually and call the sentToAppendage() method and see what it taps out.

odds are, if I've nailed the decoding, then the encoding should also be good, 
but I'll leave it up to you to figure out exactly how right I am

**note:** testing protocol 1 trumps testing protocol 2, so if you want to run testing protocol 2, be sure to deactivate testing protocol 1