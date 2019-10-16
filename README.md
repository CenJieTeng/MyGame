# MyGame


## Dependent libary
- protocol buffer [Protobuf-3.7.0]
- boost::asio [boost_1_69_0]
- lua5.3 [lua-5.3.5]

## How to start
- 1 Create new project with `cocos2d-x 3.17`
```
cd cocos2d-x-3.17.2\tools\cocos2d-console\bin

//cocos.py new -p PACK_NAME -d DIR_PATH -l cpp PROJECT_NAME
//Such as create to the desktop:
cocos.py new -p com.cocos.pack -d C:\Users\Administrator\Desktop  -l cpp MyGame
```

- 2 Replace new project `Classes` and `Resource` 
```
cd MyGame
rd /s /q Classes & rd /s /q Resource
git clone https://github.com/CenJieTeng/MyGame.git
xcopy "./MyGame/Classes" "./Classes" /F & xcopy "./MyGame/Resources" "./Resources" /F
```

- 3 Add network Module
```
git clone https://github.com/CenJieTeng/socketcore.git
Open project MyGame/proj.win32/MyGame.slh with vs2017
Add  MyGame/socketcore/libsocket to VC++ include dir
Create new filter in Classes dir and rename network
Add socketcore/libsocket/header.hpp & message.hpp & session.cpp & socket.cpp to network/
```
