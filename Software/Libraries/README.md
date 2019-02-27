# Libraries

These are shared libraries in the project.
Because of the limitations of the Arduino IDE, they have to be duplicated in each sketch folder. In order to compile the sketches, make a link from this folder to a folder called src in both the PumpController and MainController folder. These src folders are included in the .gitignore file, so only the Libraries file is in the repository.

Like: `mklink /J PumpController\src Libraries` or `ln Libraries PumpController\src`

The links themselves could be in the repository, but links in repository don't work under Windows...
