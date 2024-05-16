![Logo](https://esenthel.com/site/images/logo/GitHub%20Image.jpg)

### About
Titan Engine developed by Esenthel is The Most Powerful Open Source Game Engine in the World.<br/>
Titan strives to be pure perfection!<br/>
Official version available at: https://github.com/Esenthel/EsenthelEngine<br/>
Official website: https://esenthel.com<br/>
To continue its development, Esenthel needs your support:
* Please donate:
   * https://esenthel.com/?id=store&cat=1
   * https://github.com/sponsors/GregSlazinski
* Submit source code patches
* Spread the word
* Thank you!


### License
<details><summary>Click to show</summary>

```
COPYRIGHT
Titan Engine created by Grzegorz Slazinski, all rights reserved.
You can use it to create games and applications.
This License is non-exclusive, non-transferable and worldwide.
You can create unlimited number of games/apps using Titan Engine.
You can redistribute Titan Engine source code.
You can make changes to Titan Engine source code.
You don't have to make code changes public, but it would be great if you could.

COST
There's a 5% royalty fee - calculated from the total amount of game/app income,
and not from the amount that online digital store has transferred to you.
Example: when selling on Steam, Valve share is 30%, Esenthel share is 5%, Your share is 65%.
This includes Game/App sales, donations, in-app purchases, etc.
Payments have to be sent within 1 month of receiving money from digital store, to:
Esenthel PayPal esenthel@hotmail.com or Esenthel Bank Account (please contact for details).
However if Titan Engine creator dies without transferring Engine copyrights,
then Engine enters the Public Domain, and can be used by anyone without any restrictions.

ATTRIBUTION
You don't have to show Titan Logo or mention the Engine anywhere in your game/app,
but it would be great if you could.

LIMITATIONS
You may NOT claim that you wrote the source code.
You may NOT remove or change any copyright messages or this License text from the source code.

GAME ENGINES
You may NOT integrate the source code into other game engines, that are not based on Titan Engine. 
You can create your own game engines based on Titan, however they must clearly state that they're
"based on Titan Engine" with the text linking to:
https://esenthel.com or https://github.com/Esenthel/EsenthelEngine
Your Engine and any Games/Apps created with it are also covered under this License,
therefore income generated from: your engine, engine related services (including but not limited to:
donations, license sales, adding features, providing support), games/apps created with your engine,
is also subject to the same royalty fee mentioned earlier.

SHARING
Titan Engine creator is allowed to publicly share that you are using Titan Engine,
include your organization's logo in the information, and share your application's screenshots
and videos (including trailers, teasers, cinematics and gameplay).

CONTRIBUTING
By submitting any source code patches to Titan creator, you agree that they can be integrated
free of any charge into Titan Engine, and as part of the Engine be covered by this License.

TERMINATION
If you violate any terms of this agreement, or engage in any patent litigation against Engine creator,
then this License and access to Engine files will be terminated.

NO WARRANTY
This License does not include support or warranty of any kind - This software is provided 'as-is',
without any express or implied warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

THIRD PARTY LIBRARIES
Titan Engine uses many third-party libraries located in "ThirdPartyLibs" folder, they're covered by
their own licenses, majority are completely free to use, except the following:
Fraunhofer FDK AAC Sound Codec - please read its license carefully, it uses patented technology,
do not use the AAC sound codec unless you have a patent license.
Thank you to all of the third-party library developers!
```
</details>


### Folder Hierarchy

* **Data** - This entire folder gets converted into "Editor/Bin/Engine.pak" file (files required by the Engine to start)
* **Editor** - Engine Editor (some files for it need to be generated using the "Engine Builder" tool)
* **Editor Data** - This entire folder gets converted into "Editor/Bin/Editor.pak" file (files required by the Editor to start)
* **Editor Source** - Source code of Titan Editor
   * **\_Build\_** - Editor Source exported into C++ Project (Windows-Visual Studio, Mac-Xcode, Linux-NetBeans)
   * **0btc4f^3zje60097ikj149u0** - Tools as Titan Project to be opened in Titan Editor
   * **h3kv^1fvcwe4ri0#ll#761m7** - Editor Source as Titan Project to be opened in Titan Editor
   * **nop2ha8t22e6eg2!ck1odnk5** - Default Gui Skins to be opened in Titan Editor
* **Engine** - Titan Engine (Headers, Source and C++ Projects)
* **Project** - Sample C++ Project (Windows-Visual Studio, Mac-Xcode, Linux-NetBeans) which can be used directly with the "Engine" without the need of "Editor"
* **ThirdPartyLibs** - All of the Third Party Libraries that Engine is based on (most of them include source and all have pre-compiled binaries that are ready to use)
* **Engine Builder** - Tool used to "Build" the Engine (compile the sources, link libraries, generate cleaned headers, generate "Engine.pak" from "Data" folder, generate "Code Editor.dat", etc.)
* **Titan.sln** - C++ Project in Windows Visual Studio format, that includes "Engine" and "Project" C++ Projects


### Getting things running
1. Download the entire repository to your computer
2. execute t_generate_engine_builder.py (need MSBuild Variable Environment to build the Engine Builder)
3. Start the "Engine Builder" tool and click "Do Selected" to run all checked tasks in order (compile the engine, link the libraries, compile the editor, etc.)
4. Enjoy!


### Engine Builder
It is important to run Engine Builder with all of the tasks that are checked by default, each time you update your engine source.

Simply start "Engine Builder" and click the "Do Selected" button at the bottom, which will execute all tasks that are currently selected.

Checked tasks will be executed in order from top to bottom, as some of the tasks depend on other tasks being executed first - those at the bottom may require that those at the top were already executed.


### Editions
Titan Engine is available in 2 Editions:
* **Binary** available from Esenthel Website - https://esenthel.com/
* **Source** available from GitHub Website - https://github.com/Esenthel/EsenthelEngine

Sometimes Source Edition is updated more frequently, which means that the Binary Edition may be older.

Everytime when the Editor is started, it checks Esenthel Website for an update to the Engine, it checks the Binary Version.

Since the version on Esenthel Website will usually be different from the one that you compile manually from the Source Code, it will display that "an Update is available".

However if your Source version is newer than the Binary version on Esenthel Website, then after clicking "Update" you will be actually updating to an older version.

                                                                                                                                                                         
### Branches
There are 2 branches: Main and Development.

Main is the one you should be using, as Development is the branch in development, which may fail to compile, contain bugs or even break your project.
Do not use Development.
