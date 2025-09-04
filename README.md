# O2Physics

This repository contains the code and instructions for running ALICE O2Physics and project related stuff.

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Python](#python)
- [Contact](#contact)

## Installation
### Setting-up dependencies
Install Xcode command line tools and approve the license conditions:
```bash
sudo xcode-select --install
sudo xcodebuild -license
```

Install Homebrew and then detect any problems using:
```bash
brew doctor
```

Install the required packages and then restart the terminal:
```bash
brew install alisw/system-deps/o2-full-deps alisw/system-deps/alibuild
echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
```
### Building the packages
Create the ALICE directory and load into it:
```bash
mkdir -p ~/alice
cd ~/alice
```
Download O2Physics to develop for Run 3:
```bash
aliBuild init O2Physics@master
```
In the ```/alice``` directory, check the prerequisites:
```bash
aliDoctor O2Physics
```
Build the whole Run 3 software:
```bash
aliBuild build O2Physics
```
### Troubleshooting while building
If any errors are encountered during the building, perform the following commands:
```bash
git add .
git commit -m "<commit message>"
git pull --rebase
```
## Usage
### Entering the environment
Enter the O2Physics environment using:
```bash
alienv enter O2Physics/latest-master-o2
```
Initialise the session by getting a valid token:
```bash
alien-token-init bsuriah
```
Then enter the PEM passphrase.
Enter the directory the AO2D file is stored, then execute the analysis task using the command:
```bash
o2-analysis-<analysis-task> --aod-file <aod-filename>.root
```
To run an analysis on batch AO2D files, use the command:
```bash
o2-analysis-<analysis-task> --aod-file @aodfiles.txt
```
Once completed, exit the environment using:
```bash
exit
```

## Python
### Creating a virtual environment
To create a virtual environment within the directory of a folder, enter the command:
```bash
python3 -m venv .venv
```
