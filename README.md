If this doesnt look right click: (https://github.com/DawidNieborak/TransferViaSound)
See code on github: (https://github.com/DawidNieborak/TransferViaSound)
<div id="top"></div>
<br/><br/>

## Description

This application is the main part of the application for sending files (photos) by sound. 

The main concept is based on the fact that we have 2 functions send and receive signals. The person who sends selects a file which is copied, renames it to a random id and its post to the .NET C# server where it is saved. At the same time based on the generated id a sine wave is created and modified which reproduces the sound. The user who receives the files listens and records a short audio file, then using the FTT (https://en.wikipedia.org/wiki/Fast_Fourier_transform) algorithm we extract the individual frequencies and convert them into numbers. These numbers are our Id, sending a request to the server whether it has a file with the given id, if so, the file is returned.
<br/><br/>
## See how it works:
https://www.youtube.com/watch?v=q79WLb4jU7A

## Built With

-   Qt 
-   C++

<br/><br/>

## Author

Dawid Nieborak - dawidnieborak112@gmail.com
<br/><br/>

## License

[Mozilla Public License 2.0](https://choosealicense.com/licenses/mpl-2.0/)
