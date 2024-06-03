set(PROJECT_NAME Engine)

################################################################################
# Source groups
################################################################################
set(H
    "H/Engine.h"
)
source_group("H" FILES ${H})

set(H__Animation
    "H/Animation/Animation Keys.h"
    "H/Animation/Animation.h"
    "H/Animation/Motion.h"
    "H/Animation/Skeleton.h"
)
source_group("H\\Animation" FILES ${H__Animation})

set(H__Code
    "H/Code/CE Code Col Line.h"
    "H/Code/CE Compiler.h"
    "H/Code/CE Environment.h"
    "H/Code/CE Expression.h"
    "H/Code/CE Find.h"
    "H/Code/CE Item.h"
    "H/Code/CE Macro.h"
    "H/Code/CE Parser.h"
    "H/Code/CE Shared.h"
    "H/Code/CE Source.h"
    "H/Code/CE Symbol.h"
    "H/Code/CE Token.h"
    "H/Code/CE.h"
)
source_group("H\\Code" FILES ${H__Code})

set(H__Edit
    "H/Edit/Editor Interface.h"
    "H/Edit/Save Changes.h"
    "H/Edit/Undo.h"
    "H/Edit/Version.h"
    "H/Edit/Viewport4.h"
)
source_group("H\\Edit" FILES ${H__Edit})

set(H__File
    "H/File/Chunk.h"
    "H/File/File Text.h"
    "H/File/File.h"
    "H/File/IO.h"
    "H/File/Pak.h"
    "H/File/Xml.h"
)
source_group("H\\File" FILES ${H__File})

set(H__Game
    "H/Game/Area.h"
    "H/Game/Mini Map.h"
    "H/Game/Obj.h"
    "H/Game/Waypoint.h"
    "H/Game/World.h"
)
source_group("H\\Game" FILES ${H__Game})

set(H__Game__Objects
    "H/Game/Objects/Animatable.h"
    "H/Game/Objects/Character.h"
    "H/Game/Objects/Destructible.h"
    "H/Game/Objects/Door.h"
    "H/Game/Objects/Item.h"
    "H/Game/Objects/Kinematic.h"
    "H/Game/Objects/Obj Decal.h"
    "H/Game/Objects/Obj Light Cone.h"
    "H/Game/Objects/Obj Light Point.h"
    "H/Game/Objects/Obj Particles.h"
    "H/Game/Objects/Static.h"
)
source_group("H\\Game\\Objects" FILES ${H__Game__Objects})

set(H__Graphics
    "H/Graphics/Clouds.h"
    "H/Graphics/Color.h"
    "H/Graphics/Display Draw.h"
    "H/Graphics/Display State.h"
    "H/Graphics/Display.h"
    "H/Graphics/Effects 2D.h"
    "H/Graphics/Effects 3D.h"
    "H/Graphics/Environment.h"
    "H/Graphics/Fog.h"
    "H/Graphics/Fur.h"
    "H/Graphics/Image Atlas.h"
    "H/Graphics/Image RT.h"
    "H/Graphics/Image.h"
    "H/Graphics/Light.h"
    "H/Graphics/Material.h"
    "H/Graphics/Particle.h"
    "H/Graphics/Renderer Instancing.h"
    "H/Graphics/Renderer.h"
    "H/Graphics/Shader Compiler.h"
    "H/Graphics/Shader Main.h"
    "H/Graphics/Shader.h"
    "H/Graphics/Sky.h"
    "H/Graphics/Sun.h"
    "H/Graphics/Vertex Index Buffer Ex.h"
    "H/Graphics/Vertex Index Buffer.h"
    "H/Graphics/Water.h"
)
source_group("H\\Graphics" FILES ${H__Graphics})

set(H__Gui
    "H/Gui/Font.h"
    "H/Gui/Gui Object.h"
    "H/Gui/Gui Objects.h"
    "H/Gui/Gui Skin.h"
    "H/Gui/Gui.h"
    "H/Gui/Panel Image.h"
    "H/Gui/Panel.h"
    "H/Gui/Text Style.h"
)
source_group("H\\Gui" FILES ${H__Gui})

set(H__Gui__Objects
    "H/Gui/Objects/Button.h"
    "H/Gui/Objects/CheckBox.h"
    "H/Gui/Objects/Color Picker.h"
    "H/Gui/Objects/ComboBox.h"
    "H/Gui/Objects/Desktop.h"
    "H/Gui/Objects/Gui Custom.h"
    "H/Gui/Objects/Gui Image.h"
    "H/Gui/Objects/List.h"
    "H/Gui/Objects/Menu.h"
    "H/Gui/Objects/MenuBar.h"
    "H/Gui/Objects/Progress.h"
    "H/Gui/Objects/Property.h"
    "H/Gui/Objects/Region.h"
    "H/Gui/Objects/SlideBar.h"
    "H/Gui/Objects/Slider.h"
    "H/Gui/Objects/Tabs.h"
    "H/Gui/Objects/Text.h"
    "H/Gui/Objects/TextBox.h"
    "H/Gui/Objects/TextLine.h"
    "H/Gui/Objects/Viewport.h"
    "H/Gui/Objects/Window IO.h"
    "H/Gui/Objects/Window.h"
)
source_group("H\\Gui\\Objects" FILES ${H__Gui__Objects})

set(H__Input
    "H/Input/Input Buttons.h"
    "H/Input/Input.h"
    "H/Input/Joypad.h"
    "H/Input/Keyboard.h"
    "H/Input/Mouse.h"
    "H/Input/Touch.h"
)
source_group("H\\Input" FILES ${H__Input})

set(H__Math
    "H/Math/Calculate.h"
    "H/Math/Camera.h"
    "H/Math/Frustum.h"
    "H/Math/Math.h"
    "H/Math/Matrix.h"
    "H/Math/Number.h"
    "H/Math/Orientation.h"
    "H/Math/Quaternion.h"
    "H/Math/Random.h"
    "H/Math/Shape.h"
    "H/Math/Vector.h"
)
source_group("H\\Math" FILES ${H__Math})

set(H__Math__Shapes
    "H/Math/Shapes/Ball.h"
    "H/Math/Shapes/Box.h"
    "H/Math/Shapes/Capsule.h"
    "H/Math/Shapes/Circle.h"
    "H/Math/Shapes/Cone.h"
    "H/Math/Shapes/Edge.h"
    "H/Math/Shapes/Plane.h"
    "H/Math/Shapes/Polygon.h"
    "H/Math/Shapes/Pyramid.h"
    "H/Math/Shapes/Quad.h"
    "H/Math/Shapes/Rectangle.h"
    "H/Math/Shapes/Torus.h"
    "H/Math/Shapes/Triangle.h"
    "H/Math/Shapes/Tube.h"
)
source_group("H\\Math\\Shapes" FILES ${H__Math__Shapes})

set(H__Memory
    "H/Memory/Cache.h"
    "H/Memory/Cipher.h"
    "H/Memory/Counted Pointer.h"
    "H/Memory/Fixed Array.h"
    "H/Memory/Grid.h"
    "H/Memory/Hash.h"
    "H/Memory/Index.h"
    "H/Memory/Map.h"
    "H/Memory/Mem Block.h"
    "H/Memory/Mem Continuous Thread Safe.h"
    "H/Memory/Mem Continuous.h"
    "H/Memory/Mem Extended.h"
    "H/Memory/Mem List.h"
    "H/Memory/Mem Pointer.h"
    "H/Memory/Mem Simple.h"
    "H/Memory/Mem Temporary.h"
    "H/Memory/Mem Tree.h"
    "H/Memory/Memory.h"
    "H/Memory/Node.h"
)
source_group("H\\Memory" FILES ${H__Memory})

set(H__Memory___
    "H/Memory/_Cache.h"
    "H/Memory/_Grid.h"
    "H/Memory/_Map.h"
    "H/Memory/_Memb.h"
    "H/Memory/_Memc Thread Safe.h"
    "H/Memory/_Memc.h"
    "H/Memory/_Meml.h"
    "H/Memory/_Memx.h"
)
source_group("H\\Memory\\_" FILES ${H__Memory___})

set(H__Mesh
    "H/Mesh/Blocks.h"
    "H/Mesh/Heightmap.h"
    "H/Mesh/Material Palette.h"
    "H/Mesh/Mesh Base.h"
    "H/Mesh/Mesh Csg.h"
    "H/Mesh/Mesh Cuts.h"
    "H/Mesh/Mesh Group.h"
    "H/Mesh/Mesh Import.h"
    "H/Mesh/Mesh Lod.h"
    "H/Mesh/Mesh Mesh.h"
    "H/Mesh/Mesh Overlay.h"
    "H/Mesh/Mesh Part.h"
    "H/Mesh/Mesh Render.h"
    "H/Mesh/Voxel.h"
)
source_group("H\\Mesh" FILES ${H__Mesh})

set(H__Misc
    "H/Misc/Application.h"
    "H/Misc/Callback.h"
    "H/Misc/Character.h"
    "H/Misc/Compress.h"
    "H/Misc/Cpu.h"
    "H/Misc/Date Time.h"
    "H/Misc/DLL.h"
    "H/Misc/Enum.h"
    "H/Misc/Member.h"
    "H/Misc/Misc.h"
    "H/Misc/MP4.h"
    "H/Misc/Object.h"
    "H/Misc/Param.h"
    "H/Misc/Pathfind 2D.h"
    "H/Misc/Pathfind.h"
    "H/Misc/Printer.h"
    "H/Misc/Reference.h"
    "H/Misc/Sort.h"
    "H/Misc/States.h"
    "H/Misc/String Borrowed.h"
    "H/Misc/String Functions.h"
    "H/Misc/String.h"
    "H/Misc/String8.h"
    "H/Misc/Thread.h"
    "H/Misc/Timer.h"
    "H/Misc/Translation.h"
    "H/Misc/UID.h"
    "H/Misc/Video.h"
    "H/Misc/Windows.h"
)
source_group("H\\Misc" FILES ${H__Misc})

set(H__Net
    "H/Net/Advertisement.h"
    "H/Net/Connection.h"
    "H/Net/Download.h"
    "H/Net/Esenthel Store.h"
    "H/Net/Facebook.h"
    "H/Net/Ftp.h"
    "H/Net/Game Analytics.h"
    "H/Net/Internet Cache.h"
    "H/Net/Interpolator.h"
    "H/Net/Patcher.h"
    "H/Net/Socket.h"
    "H/Net/Sql.h"
    "H/Net/Steam.h"
    "H/Net/Store.h"
    "H/Net/Xbox Live.h"
)
source_group("H\\Net" FILES ${H__Net})

set(H__Physics
    "H/Physics/Actor.h"
    "H/Physics/Aggregate.h"
    "H/Physics/Cloth.h"
    "H/Physics/Controller.h"
    "H/Physics/Destruct Mesh.h"
    "H/Physics/Grab.h"
    "H/Physics/Joint.h"
    "H/Physics/Phys Body.h"
    "H/Physics/Phys Material.h"
    "H/Physics/Physics.h"
    "H/Physics/Ragdoll.h"
    "H/Physics/Vehicle.h"
)
source_group("H\\Physics" FILES ${H__Physics})

set(H__Platforms
    "../NintendoSwitch/Switch.h"
    "H/Platforms/Android.h"
)
source_group("H\\Platforms" FILES ${H__Platforms})

set(H__Sound
    "H/Sound/Music.h"
    "H/Sound/Sound Buffer.h"
    "H/Sound/Sound Extra.h"
    "H/Sound/Sound Stream.h"
    "H/Sound/Sound.h"
)
source_group("H\\Sound" FILES ${H__Sound})

set(H___
    "H/_/defines.h"
    "H/_/headers.h"
    "H/_/inline.h"
    "H/_/structs.h"
    "H/_/types.h"
    "stdafx.cpp"
    "stdafx.h"
)
source_group("H\\_" FILES ${H___})

set(H_____OpenGL
#    "../ThirdPartyLibs/GL/glew.cpp"
)
source_group("H\\_\\OpenGL" FILES ${H_____OpenGL})

set(Source__Animation
    "Source/Animation/Animated Skeleton.cpp"
    "Source/Animation/Animation.cpp"
    "Source/Animation/Motion.cpp"
    "Source/Animation/Skeleton.cpp"
)
source_group("Source\\Animation" FILES ${Source__Animation})

set(Source__Code
#    "Source/Code/CE Code Col Line.cpp"
#    "Source/Code/CE Editor C++.cpp"
#    "Source/Code/CE Editor GUI.cpp"
#    "Source/Code/CE Editor IO.cpp"
#    "Source/Code/CE Editor Parser.cpp"
#    "Source/Code/CE Editor Sources.cpp"
#    "Source/Code/CE Find.cpp"
#    "Source/Code/CE Item.cpp"
#    "Source/Code/CE Macro.cpp"
#    "Source/Code/CE Parser.cpp"
#    "Source/Code/CE Source C++.cpp"
#    "Source/Code/CE Source Edit.cpp"
#    "Source/Code/CE Source GUI.cpp"
#    "Source/Code/CE Source Preprocessor.cpp"
#    "Source/Code/CE Source Suggestions.cpp"
#    "Source/Code/CE Source.cpp"
#    "Source/Code/CE Symbol.cpp"
#    "Source/Code/CE Token.cpp"
#    "Source/Code/CE.cpp"
)
source_group("Source\\Code" FILES ${Source__Code})

set(Source__Code__Compile
#    "Source/Code/Compile/CE Command.cpp"
#    "Source/Code/Compile/CE Compiler.cpp"
#    "Source/Code/Compile/CE Environment.cpp"
#    "Source/Code/Compile/CE Expression.cpp"
#    "Source/Code/Compile/CE Func List.cpp"
#    "Source/Code/Compile/CE Func List.h"
)
source_group("Source\\Code\\Compile" FILES ${Source__Code__Compile})

set(Source__Edit
    "Source/Edit/Editor Interface.cpp"
    "Source/Edit/Save Changes.cpp"
    "Source/Edit/Undo.cpp"
    "Source/Edit/Version.cpp"
    "Source/Edit/Viewport4.cpp"
)
source_group("Source\\Edit" FILES ${Source__Edit})

set(Source__File
    "Source/File/Chunk.cpp"
    "Source/File/File Text.cpp"
    "Source/File/File.cpp"
    "Source/File/IO.cpp"
    "Source/File/Pak.cpp"
    "Source/File/Xml.cpp"
)
source_group("Source\\File" FILES ${Source__File})

set(Source__Game
    "Source/Game/Area.cpp"
    "Source/Game/Mini Map.cpp"
    "Source/Game/Obj.cpp"
    "Source/Game/Path 2D Walker.cpp"
    "Source/Game/Waypoint.cpp"
    "Source/Game/World Draw.cpp"
    "Source/Game/World Object.cpp"
    "Source/Game/World Path 2D.cpp"
    "Source/Game/World Terrain.cpp"
    "Source/Game/World.cpp"
)
source_group("Source\\Game" FILES ${Source__Game})

set(Source__Game__Objects
    "Source/Game/Objects/Animatable.cpp"
    "Source/Game/Objects/Character Action.cpp"
    "Source/Game/Objects/Character Animation.cpp"
    "Source/Game/Objects/Character Item.cpp"
    "Source/Game/Objects/Character Ragdoll.cpp"
    "Source/Game/Objects/Character.cpp"
    "Source/Game/Objects/Destructible.cpp"
    "Source/Game/Objects/Door.cpp"
    "Source/Game/Objects/Item.cpp"
    "Source/Game/Objects/Kinematic.cpp"
    "Source/Game/Objects/Obj Decal.cpp"
    "Source/Game/Objects/Obj Light Cone.cpp"
    "Source/Game/Objects/Obj Light Point.cpp"
    "Source/Game/Objects/Obj Particles.cpp"
    "Source/Game/Objects/Static.cpp"
)
source_group("Source\\Game\\Objects" FILES ${Source__Game__Objects})

set(Source__Graphics
    "Source/Graphics/Clouds.cpp"
    "Source/Graphics/Color.cpp"
    "Source/Graphics/Decal.cpp"
    "Source/Graphics/Display Draw.cpp"
    "Source/Graphics/Display State.cpp"
    "Source/Graphics/Display Viewport.cpp"
    "Source/Graphics/Display.cpp"
    "Source/Graphics/Effects 2D.cpp"
    "Source/Graphics/Effects 3D.cpp"
    "Source/Graphics/Environment.cpp"
    "Source/Graphics/Fog.cpp"
    "Source/Graphics/Fur.cpp"
    "Source/Graphics/Image Atlas.cpp"
    "Source/Graphics/Image Filter Waifu.cpp"
    "Source/Graphics/Image IO.cpp"
    "Source/Graphics/Image Operations.cpp"
    "Source/Graphics/Image Pixel.cpp"
    "Source/Graphics/Image RT.cpp"
    "Source/Graphics/Image.cpp"
    "Source/Graphics/lights/Light.cpp"
    "Source/Graphics/Material.cpp"
    "Source/Graphics/Particle.cpp"
    "Source/Graphics/Renderer Instancing.cpp"
    "Source/Graphics/Renderer Targets.cpp"
    "Source/Graphics/Renderer.cpp"
    "Source/Graphics/Shader Compiler.cpp"
    "Source/Graphics/Shader Compilers.cpp"
    "Source/Graphics/Shader Get.cpp"
    "Source/Graphics/Shader Main.cpp"
    "Source/Graphics/Shader.cpp"
    "Source/Graphics/Sky.cpp"
    "Source/Graphics/Sun.cpp"
    "Source/Graphics/Vertex Index Buffer.cpp"
    "Source/Graphics/Water.cpp"
)
source_group("Source\\Graphics" FILES ${Source__Graphics})

set(Source__Graphics__Import
    "Source/Graphics/Import/ASTC Compress.cpp"
    "Source/Graphics/Import/ASTC.cpp"
    "Source/Graphics/Import/AVIF Compress.cpp"
    "Source/Graphics/Import/AVIF.cpp"
    "Source/Graphics/Import/BC Compress.cpp"
    "Source/Graphics/Import/BC.cpp"
    "Source/Graphics/Import/BC.h"
    "Source/Graphics/Import/BMP.cpp"
    "Source/Graphics/Import/DDS.cpp"
    "Source/Graphics/Import/ETC Compress.cpp"
    "Source/Graphics/Import/ETC.cpp"
    "Source/Graphics/Import/ETC.h"
    "Source/Graphics/Import/HDR.cpp"
    "Source/Graphics/Import/HEIF.cpp"
    "Source/Graphics/Import/ICO.cpp"
    "Source/Graphics/Import/JPEG XL Compress.cpp"
    "Source/Graphics/Import/JPEG XL.cpp"
    "Source/Graphics/Import/JPEG.cpp"
    "Source/Graphics/Import/PNG.cpp"
    "Source/Graphics/Import/PSD.cpp"
    "Source/Graphics/Import/PVRTC Compress.cpp"
    "Source/Graphics/Import/PVRTC.cpp"
    "Source/Graphics/Import/TGA.cpp"
    "Source/Graphics/Import/TIF.cpp"
    "Source/Graphics/Import/WEBP.cpp"
)
source_group("Source\\Graphics\\Import" FILES ${Source__Graphics__Import})

set(Source__Gui
    "Source/Gui/Font.cpp"
    "Source/Gui/Gui Object.cpp"
    "Source/Gui/Gui Objects.cpp"
    "Source/Gui/Gui Skin.cpp"
    "Source/Gui/Gui.cpp"
    "Source/Gui/Panel Image.cpp"
    "Source/Gui/Panel.cpp"
    "Source/Gui/Text Style.cpp"
)
source_group("Source\\Gui" FILES ${Source__Gui})

set(Source__Gui__Objects
    "Source/Gui/Objects/Button.cpp"
    "Source/Gui/Objects/CheckBox.cpp"
    "Source/Gui/Objects/Color Picker.cpp"
    "Source/Gui/Objects/ComboBox.cpp"
    "Source/Gui/Objects/Desktop.cpp"
    "Source/Gui/Objects/Gui Custom.cpp"
    "Source/Gui/Objects/Gui Image.cpp"
    "Source/Gui/Objects/List.cpp"
    "Source/Gui/Objects/Menu.cpp"
    "Source/Gui/Objects/MenuBar.cpp"
    "Source/Gui/Objects/Progress.cpp"
    "Source/Gui/Objects/Property.cpp"
    "Source/Gui/Objects/Region.cpp"
    "Source/Gui/Objects/SlideBar.cpp"
    "Source/Gui/Objects/Slider.cpp"
    "Source/Gui/Objects/Tabs.cpp"
    "Source/Gui/Objects/Text.cpp"
    "Source/Gui/Objects/TextBox.cpp"
    "Source/Gui/Objects/TextLine.cpp"
    "Source/Gui/Objects/Viewport.cpp"
    "Source/Gui/Objects/Window IO.cpp"
    "Source/Gui/Objects/Window.cpp"
)
source_group("Source\\Gui\\Objects" FILES ${Source__Gui__Objects})

set(Source__Input
    "Source/Input/Input.cpp"
    "Source/Input/Joypad.cpp"
    "Source/Input/Keyboard.cpp"
    "Source/Input/Mouse.cpp"
    "Source/Input/Oculus Rift.cpp"
    "Source/Input/OpenVR.cpp"
    "Source/Input/Touch.cpp"
    "Source/Input/Virtual Reality.cpp"
)
source_group("Source\\Input" FILES ${Source__Input})

set(Source__Math
    "Source/Math/Calculate.cpp"
    "Source/Math/Camera.cpp"
    "Source/Math/Frustum.cpp"
    "Source/Math/Math.cpp"
    "Source/Math/Matrix.cpp"
    "Source/Math/Number.cpp"
    "Source/Math/Orientation.cpp"
    "Source/Math/Quaternion.cpp"
    "Source/Math/Random.cpp"
    "Source/Math/Shape.cpp"
    "Source/Math/Vector.cpp"
)
source_group("Source\\Math" FILES ${Source__Math})

set(Source__Math__Shapes
    "Source/Math/Shapes/Ball.cpp"
    "Source/Math/Shapes/Box.cpp"
    "Source/Math/Shapes/Capsule.cpp"
    "Source/Math/Shapes/Circle.cpp"
    "Source/Math/Shapes/Cone.cpp"
    "Source/Math/Shapes/Edge.cpp"
    "Source/Math/Shapes/Plane.cpp"
    "Source/Math/Shapes/Polygon.cpp"
    "Source/Math/Shapes/Pyramid.cpp"
    "Source/Math/Shapes/Quad.cpp"
    "Source/Math/Shapes/Rectangle.cpp"
    "Source/Math/Shapes/Torus.cpp"
    "Source/Math/Shapes/Triangle.cpp"
    "Source/Math/Shapes/Tube.cpp"
)
source_group("Source\\Math\\Shapes" FILES ${Source__Math__Shapes})

set(Source__Memory
    "Source/Memory/Cache.cpp"
    "Source/Memory/Cipher.cpp"
    "Source/Memory/Counted Pointer.cpp"
    "Source/Memory/Grid.cpp"
    "Source/Memory/Hash.cpp"
    "Source/Memory/Index.cpp"
    "Source/Memory/Map.cpp"
    "Source/Memory/Mem Block.cpp"
    "Source/Memory/Mem Continuous.cpp"
    "Source/Memory/Mem Extended.cpp"
    "Source/Memory/Mem List.cpp"
    "Source/Memory/Mem Tree.cpp"
    "Source/Memory/Memory.cpp"
)
source_group("Source\\Memory" FILES ${Source__Memory})

set(Source__Mesh
    "Source/Mesh/Blocks.cpp"
    "Source/Mesh/Heightmap.cpp"
    "Source/Mesh/Material Palette.cpp"
    "Source/Mesh/Mesh Base.cpp"
    "Source/Mesh/Mesh Cuts.cpp"
    "Source/Mesh/Mesh Draw.cpp"
    "Source/Mesh/Mesh Group.cpp"
    "Source/Mesh/Mesh IO.cpp"
    "Source/Mesh/Mesh Lod.cpp"
    "Source/Mesh/Mesh Mesh.cpp"
    "Source/Mesh/Mesh Optimize.cpp"
    "Source/Mesh/Mesh Overlay.cpp"
    "Source/Mesh/Mesh Part.cpp"
    "Source/Mesh/Mesh Render.cpp"
    "Source/Mesh/Mesh Simplify.cpp"
    "Source/Mesh/Voxel.cpp"
)
source_group("Source\\Mesh" FILES ${Source__Mesh})

set(Source__Mesh__Import
    "Source/Mesh/Import/Export OBJ.cpp"
    "Source/Mesh/Import/Import 3DS.cpp"
    "Source/Mesh/Import/Import ASE.cpp"
    "Source/Mesh/Import/Import B3D.cpp"
    "Source/Mesh/Import/Import BVH.cpp"
    "Source/Mesh/Import/Import DAE.cpp"
    "Source/Mesh/Import/Import FBX.cpp"
    "Source/Mesh/Import/Import MS3D.cpp"
    "Source/Mesh/Import/Import OBJ.cpp"
    "Source/Mesh/Import/Import PSK PSA.cpp"
    "Source/Mesh/Import/Import XPS.cpp"
)
source_group("Source\\Mesh\\Import" FILES ${Source__Mesh__Import})

set(Source__Mesh__MeshBase
    "Source/Mesh/MeshBase/Mshb Add Remove.cpp"
    "Source/Mesh/MeshBase/Mshb Convert.cpp"
    "Source/Mesh/MeshBase/Mshb Create.cpp"
    "Source/Mesh/MeshBase/Mshb Csg 2D.cpp"
    "Source/Mesh/MeshBase/Mshb Csg.cpp"
    "Source/Mesh/MeshBase/Mshb Fix.cpp"
    "Source/Mesh/MeshBase/Mshb Join Split.cpp"
    "Source/Mesh/MeshBase/Mshb Link.cpp"
    "Source/Mesh/MeshBase/Mshb Operations.cpp"
    "Source/Mesh/MeshBase/Mshb Set.cpp"
    "Source/Mesh/MeshBase/Mshb Texturize.cpp"
    "Source/Mesh/MeshBase/Mshb Transform.cpp"
)
source_group("Source\\Mesh\\MeshBase" FILES ${Source__Mesh__MeshBase})

set(Source__Misc
    "Source/Misc/Application.cpp"
    "Source/Misc/Call Stack.cpp"
    "Source/Misc/Callback.cpp"
    "Source/Misc/CMDExecutor.cpp"
    "Source/Misc/Compress.cpp"
    "Source/Misc/Cpu.cpp"
    "Source/Misc/Date Time.cpp"
    "Source/Misc/DLL.cpp"
    "Source/Misc/Enum.cpp"
    "Source/Misc/InitLogThreadAndDisableCrashDump.cpp"
    "Source/Misc/Member.cpp"
    "Source/Misc/Misc.cpp"
    "Source/Misc/MP4.cpp"
    "Source/Misc/Object.cpp"
    "Source/Misc/Param.cpp"
    "Source/Misc/Pathfind 2D.cpp"
    "Source/Misc/Pathfind.cpp"
    "Source/Misc/Printer.cpp"
    "Source/Misc/Process.cpp"
    "Source/Misc/Sort.cpp"
    "Source/Misc/States.cpp"
    "Source/Misc/String.cpp"
    "Source/Misc/Thread.cpp"
    "Source/Misc/Timer.cpp"
    "Source/Misc/Translation.cpp"
    "Source/Misc/Video.cpp"
    "Source/Misc/Windows.cpp"
)
source_group("Source\\Misc" FILES ${Source__Misc})

set(Source__Net
    "Source/Net/AdMob.cpp"
    "Source/Net/Chartboost.cpp"
    "Source/Net/Connection.cpp"
    "Source/Net/Download.cpp"
    "Source/Net/Esenthel Store.cpp"
    "Source/Net/Facebook.cpp"
    "Source/Net/Ftp.cpp"
    "Source/Net/Game Analytics.cpp"
    "Source/Net/Internet Cache.cpp"
    "Source/Net/Interpolator.cpp"
    "Source/Net/Patcher.cpp"
    "Source/Net/Socket.cpp"
    "Source/Net/Sql.cpp"
    "Source/Net/Steam.cpp"
    "Source/Net/Store.cpp"
    "Source/Net/Xbox Live.cpp"
)
source_group("Source\\Net" FILES ${Source__Net})

set(Source__Physics
    "Source/Physics/Actor Create.cpp"
    "Source/Physics/Actor.cpp"
    "Source/Physics/Aggregate.cpp"
    "Source/Physics/Cloth.cpp"
    "Source/Physics/Controller.cpp"
    "Source/Physics/Destruct Mesh.cpp"
    "Source/Physics/Grab.cpp"
    "Source/Physics/Joint.cpp"
    "Source/Physics/Phys Body.cpp"
    "Source/Physics/Phys Material.cpp"
    "Source/Physics/Physics Draw.cpp"
    "Source/Physics/Physics Test.cpp"
    "Source/Physics/Physics.cpp"
    "Source/Physics/Ragdoll.cpp"
    "Source/Physics/Vehicle.cpp"
)
source_group("Source\\Physics" FILES ${Source__Physics})

set(Source__Platforms__Android
    "../Editor Data/Code/Android/app/src/main/AndroidManifest.xml"
    "../Editor Data/Code/Android/app/src/main/java/EsenthelActivity.java"
    "../Editor Data/Code/Android/app/src/main/java/Native.java"
#    "Source/Platforms/Android/Android-C.c"
#    "Source/Platforms/Android/Android.cpp"
)
source_group("Source\\Platforms\\Android" FILES ${Source__Platforms__Android})

set(Source__Platforms__Mac
    "Source/Platforms/Mac/MyApplication.h"
    "Source/Platforms/Mac/MyApplication.mm"
    "Source/Platforms/Mac/MyOpenGLView.h"
    "Source/Platforms/Mac/MyOpenGLView.mm"
    "Source/Platforms/Mac/MyWindow.h"
    "Source/Platforms/Mac/MyWindow.mm"
)
source_group("Source\\Platforms\\Mac" FILES ${Source__Platforms__Mac})

set(Source__Platforms__Switch
#    "../NintendoSwitch/Switch.cpp"
)
source_group("Source\\Platforms\\Switch" FILES ${Source__Platforms__Switch})

set(Source__Platforms__Web
#    "Source/Platforms/Web/Web.cpp"
)
source_group("Source\\Platforms\\Web" FILES ${Source__Platforms__Web})

set(Source__Platforms__Windows
#    "Source/Platforms/Windows/Windows Platform.cpp"
)
source_group("Source\\Platforms\\Windows" FILES ${Source__Platforms__Windows})

set(Source__Platforms__iOS
    "Source/Platforms/iOS/EAGLView.h"
    "Source/Platforms/iOS/EAGLView.mm"
    "Source/Platforms/iOS/IAPManager.h"
    "Source/Platforms/iOS/IAPManager.mm"
    "Source/Platforms/iOS/iOS.h"
    "Source/Platforms/iOS/iOSAppDelegate.h"
    "Source/Platforms/iOS/iOSAppDelegate.mm"
    "Source/Platforms/iOS/MyViewController.h"
    "Source/Platforms/iOS/MyViewController.mm"
)
source_group("Source\\Platforms\\iOS" FILES ${Source__Platforms__iOS})

set(Source__Shaders
    "Source/Shaders/!Header CPU.h"
    "Source/Shaders/!Header.h"
    "Source/Shaders/!Set Prec Default.h"
    "Source/Shaders/!Set Prec High.h"
    "Source/Shaders/!Set Prec Image.h"
    "Source/Shaders/!Set Prec Low.h"
    "Source/Shaders/!Set Prec Struct.h"
#    "Source/Shaders/Ambient Occlusion.cpp"
    "Source/Shaders/Ambient Occlusion.h"
#    "Source/Shaders/Atmosphere.cpp"
#    "Source/Shaders/Bloom.cpp"
    "Source/Shaders/Bloom.h"
#    "Source/Shaders/Blur.cpp"
#    "Source/Shaders/Cubic.cpp"
    "Source/Shaders/Cubic.h"
#    "Source/Shaders/Depth of Field.cpp"
#    "Source/Shaders/DX10+ Input Layout.cpp"
#    "Source/Shaders/Effects 2D.cpp"
#    "Source/Shaders/Effects 3D.cpp"
#    "Source/Shaders/FidelityFX/Fidelity FX.cpp"
#    "Source/Shaders/Fog.cpp"
    "Source/Shaders/Fog.h"
#    "Source/Shaders/Font.cpp"
#    "Source/Shaders/FXAA.cpp"
    "Source/Shaders/FXAA.h"
#    "Source/Shaders/Hdr.cpp"
    "Source/Shaders/Hdr.h"
#    "Source/Shaders/Layered Clouds.cpp"
    "Source/Shaders/Layered Clouds.h"
#    "Source/Shaders/Light Apply.cpp"
    "Source/Shaders/Light Apply.h"
#    "Source/Shaders/Light.cpp"
#    "Source/Shaders/Main.cpp"
#    "Source/Shaders/Motion Blur.cpp"
    "Source/Shaders/Motion Blur.h"
#    "Source/Shaders/Outline.cpp"
#    "Source/Shaders/Particles.cpp"
#    "Source/Shaders/Shadow.cpp"
#    "Source/Shaders/Sky.cpp"
    "Source/Shaders/Sky.h"
#    "Source/Shaders/SMAA.cpp"
    "Source/Shaders/SMAA.h"
#    "Source/Shaders/Sun.cpp"
#    "Source/Shaders/Temporal.cpp"
    "Source/Shaders/Temporal.h"
#    "Source/Shaders/Video.cpp"
#    "Source/Shaders/Volumetric Clouds.cpp"
#    "Source/Shaders/Volumetric Lights.cpp"
#    "Source/Shaders/Water.cpp"
    "Source/Shaders/Water.h"
#    "Source/Shaders/World Editor.cpp"
)
source_group("Source\\Shaders" FILES ${Source__Shaders})

set(Source__Shaders__3D
#    "Source/Shaders/Behind.cpp"
#    "Source/Shaders/Blend Light.cpp"
#    "Source/Shaders/Blend.cpp"
#    "Source/Shaders/Bone Highlight.cpp"
#    "Source/Shaders/Deferred.cpp"
#    "Source/Shaders/Early Z.cpp"
#    "Source/Shaders/Emissive.cpp"
#    "Source/Shaders/Forward.cpp"
#    "Source/Shaders/Fur.cpp"
    "Source/Shaders/Fur.h"
#    "Source/Shaders/Mesh Overlay.cpp"
    "Source/Shaders/Mesh Overlay.h"
#    "Source/Shaders/Overlay.cpp"
#    "Source/Shaders/Position.cpp"
#    "Source/Shaders/Set Color.cpp"
)
source_group("Source\\Shaders\\3D" FILES ${Source__Shaders__3D})

set(Source__Sound
    "Source/Sound/App Volume.cpp"
    "Source/Sound/Music.cpp"
    "Source/Sound/Sound Buffer.cpp"
    "Source/Sound/Sound Record.cpp"
    "Source/Sound/Sound Resample.cpp"
    "Source/Sound/Sound Stream.cpp"
    "Source/Sound/Sound.cpp"
    "Source/Sound/Spectrometer.cpp"
    "Source/Sound/Vorbis Encoder.cpp"
)
source_group("Source\\Sound" FILES ${Source__Sound})

set(ALL_FILES
    ${H}
    ${H__Animation}
    ${H__Code}
    ${H__Edit}
    ${H__File}
    ${H__Game}
    ${H__Game__Objects}
    ${H__Graphics}
    ${H__Gui}
    ${H__Gui__Objects}
    ${H__Input}
    ${H__Math}
    ${H__Math__Shapes}
    ${H__Memory}
    ${H__Memory___}
    ${H__Mesh}
    ${H__Misc}
    ${H__Net}
    ${H__Physics}
    ${H__Platforms}
    ${H__Sound}
    ${H___}
    ${H_____OpenGL}
    ${Source__Animation}
    ${Source__Code}
    ${Source__Code__Compile}
    ${Source__Edit}
    ${Source__File}
    ${Source__Game}
    ${Source__Game__Objects}
    ${Source__Graphics}
    ${Source__Graphics__Import}
    ${Source__Gui}
    ${Source__Gui__Objects}
    ${Source__Input}
    ${Source__Math}
    ${Source__Math__Shapes}
    ${Source__Memory}
    ${Source__Mesh}
    ${Source__Mesh__Import}
    ${Source__Mesh__MeshBase}
    ${Source__Misc}
    ${Source__Net}
    ${Source__Physics}
    ${Source__Platforms__Android}
    ${Source__Platforms__Mac}
    ${Source__Platforms__Switch}
    ${Source__Platforms__Web}
    ${Source__Platforms__Windows}
    ${Source__Platforms__iOS}
    ${Source__Shaders}
    ${Source__Shaders__3D}
    ${Source__Sound}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h>"
)

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
################################################################################
# Includes for CMake from *.props
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    use_props(${PROJECT_NAME} DebugDX11            "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} DebugGL              "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} DebugUniversalDX11   "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} ReleaseDX11          "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} ReleaseGL            "ImportNintendoSdk.cmake")
    use_props(${PROJECT_NAME} ReleaseUniversalDX11 "ImportNintendoSdk.cmake")
endif()

set(ROOT_NAMESPACE Engine)

################################################################################
# Target name
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "1) 64 bit")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUGDX11            "${PROJECT_NAME}Debug64DX11"
        TARGET_NAME_DEBUGGL              "${PROJECT_NAME}Debug64GL"
        TARGET_NAME_DEBUGUNIVERSALDX11   "${PROJECT_NAME}DebugUniversal64DX11"
        TARGET_NAME_RELEASEDX11          "${PROJECT_NAME}64DX11"
        TARGET_NAME_RELEASEGL            "${PROJECT_NAME}64GL"
        TARGET_NAME_RELEASEUNIVERSALDX11 "${PROJECT_NAME}Universal64DX11"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUGDX11            "${PROJECT_NAME}Debug32DX11"
        TARGET_NAME_DEBUGGL              "${PROJECT_NAME}Debug32GL"
        TARGET_NAME_DEBUGUNIVERSALDX11   "${PROJECT_NAME}DebugUniversal32DX11"
        TARGET_NAME_RELEASEDX11          "${PROJECT_NAME}32DX11"
        TARGET_NAME_RELEASEGL            "${PROJECT_NAME}32GL"
        TARGET_NAME_RELEASEUNIVERSALDX11 "${PROJECT_NAME}Universal32DX11"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUGDX11            "${PROJECT_NAME}DebugArm32DX11"
        TARGET_NAME_DEBUGGL              "${PROJECT_NAME}DebugArm32GL"
        TARGET_NAME_DEBUGUNIVERSALDX11   "${PROJECT_NAME}DebugUniversalArm32DX11"
        TARGET_NAME_RELEASEDX11          "${PROJECT_NAME}Arm32DX11"
        TARGET_NAME_RELEASEGL            "${PROJECT_NAME}Arm32GL"
        TARGET_NAME_RELEASEUNIVERSALDX11 "${PROJECT_NAME}UniversalArm32DX11"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUGDX11            "${PROJECT_NAME}Debug"
        TARGET_NAME_DEBUGGL              "${PROJECT_NAME}Debug"
        TARGET_NAME_DEBUGUNIVERSALDX11   "${PROJECT_NAME}Debug"
        TARGET_NAME_RELEASEDX11          "${PROJECT_NAME}"
        TARGET_NAME_RELEASEGL            "${PROJECT_NAME}"
        TARGET_NAME_RELEASEUNIVERSALDX11 "${PROJECT_NAME}"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUGDX11            "${PROJECT_NAME}DebugNintendoSwitch"
        TARGET_NAME_DEBUGGL              "${PROJECT_NAME}DebugNintendoSwitch"
        TARGET_NAME_DEBUGUNIVERSALDX11   "${PROJECT_NAME}DebugNintendoSwitch"
        TARGET_NAME_RELEASEDX11          "${PROJECT_NAME}NintendoSwitch"
        TARGET_NAME_RELEASEGL            "${PROJECT_NAME}NintendoSwitch"
        TARGET_NAME_RELEASEUNIVERSALDX11 "${PROJECT_NAME}NintendoSwitch"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        TARGET_NAME_DEBUGDX11            "${PROJECT_NAME}DebugAndroid"
        TARGET_NAME_DEBUGGL              "${PROJECT_NAME}DebugAndroid"
        TARGET_NAME_DEBUGUNIVERSALDX11   "${PROJECT_NAME}DebugAndroid"
        TARGET_NAME_RELEASEDX11          "${PROJECT_NAME}Android"
        TARGET_NAME_RELEASEGL            "${PROJECT_NAME}Android"
        TARGET_NAME_RELEASEUNIVERSALDX11 "${PROJECT_NAME}Android"
    )
endif()
################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "1) 64 bit")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUGDX11            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGGL              "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_DEBUGUNIVERSALDX11   "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEDX11          "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEGL            "${CMAKE_CURRENT_SOURCE_DIR}/"
        OUTPUT_DIRECTORY_RELEASEUNIVERSALDX11 "${CMAKE_CURRENT_SOURCE_DIR}/"
    )
endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "1) 64 bit")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION                      "FALSE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_DEBUGDX11            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGGL              "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_DEBUGUNIVERSALDX11   "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEDX11          "TRUE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEGL            "FALSE"
        INTERPROCEDURAL_OPTIMIZATION_RELEASEUNIVERSALDX11 "TRUE"
    )
endif()
################################################################################
# MSVC runtime library
################################################################################
get_property(MSVC_RUNTIME_LIBRARY_DEFAULT TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY)
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "1) 64 bit")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        MultiThreaded
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    string(CONCAT "MSVC_RUNTIME_LIBRARY_STR"
        $<$<CONFIG:DebugDX11>:
            MultiThreaded
        >
        $<$<CONFIG:DebugGL>:
            MultiThreaded
        >
        $<$<CONFIG:DebugUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<CONFIG:ReleaseDX11>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseGL>:
            MultiThreaded
        >
        $<$<CONFIG:ReleaseUniversalDX11>:
            MultiThreadedDLL
        >
        $<$<NOT:$<OR:$<CONFIG:DebugDX11>,$<CONFIG:DebugGL>,$<CONFIG:DebugUniversalDX11>,$<CONFIG:ReleaseDX11>,$<CONFIG:ReleaseGL>,$<CONFIG:ReleaseUniversalDX11>>>:${MSVC_RUNTIME_LIBRARY_DEFAULT}>
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY ${MSVC_RUNTIME_LIBRARY_STR})

################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "1) 64 bit")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/H/_;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/JpegXL/lib/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Xbox Live/2018.6.20181010.2;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/pxshared/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/foundation/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/common/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/geomutils/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Imath;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Iex;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Half;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/OpenEXR/config;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/IlmBase/config"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/H/_;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/JpegXL/lib/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Xbox Live/2018.6.20181010.2;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/pxshared/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/foundation/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/common/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/geomutils/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Imath;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Iex;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Half;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/OpenEXR/config;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/IlmBase/config"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/H/_;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/JpegXL/lib/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Xbox Live/2018.6.20181010.2;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/pxshared/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/foundation/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/common/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/geomutils/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Imath;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Iex;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Half;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/OpenEXR/config;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/IlmBase/config"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/H/_;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/H/_;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/JpegXL/lib/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Xbox Live/2018.6.20181010.2;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/pxshared/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/foundation/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/common/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/geomutils/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../NintendoSwitch/ThirdPartyLibs/PhysX/physx/source/foundation/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../NintendoSwitch/ThirdPartyLibs/PhysX/pxshared/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Imath;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Iex;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Half;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/OpenEXR/config;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/IlmBase/config"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/H/_;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FreeType/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/DirectX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Zlib;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Opus/file/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Ogg/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Theora/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Vorbis/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FDK-AAC/lib/libSYS/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/FBX;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Bullet/lib/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/VP/libvpx/third_party/libwebm;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/JpegXL/lib/lib/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/Xbox Live/2018.6.20181010.2;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/pxshared/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/foundation/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/common/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/PhysX/physx/source/geomutils/src;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Imath;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Iex;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Half;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/OpenEXR/config;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/IlmBase/config;"
        "$ENV{VS_NdkRoot}/sources/android"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "1) 64 bit")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0;"
            "DX11"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:DebugDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugGL>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:DebugUniversalDX11>:"
            "DEBUG=1"
        ">"
        "$<$<CONFIG:ReleaseDX11>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseGL>:"
            "DEBUG=0"
        ">"
        "$<$<CONFIG:ReleaseUniversalDX11>:"
            "DEBUG=0"
        ">"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "1) 64 bit")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /MP;
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /Gd;
            /fp:fast;
            /GR;
            /wd4018;
            /wd4244;
            /wd4312;
            /wd4521;
            /wd4703;
            /J;
            /permissive;
            -Zm512;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /ZI
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /ZI
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /ZI
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /MP;
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /arch:SSE2;
            /Gd;
            /fp:fast;
            /GR;
            /wd4018;
            /wd4244;
            /wd4312;
            /wd4521;
            /wd4703;
            /J;
            /permissive;
            -Zm512;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /ZI
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /MP;
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /Gd;
            /fp:fast;
            /GR;
            /wd4018;
            /wd4244;
            /wd4312;
            /wd4521;
            /wd4703;
            /J;
            /permissive;
            -Zm512;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /Od;
                /Ob0;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /Od;
                /Ob0;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /Od;
                /Ob0;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /O2;
                /Ob2;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            $<$<CONFIG:ReleaseGL>:
                /O2;
                /Ob2;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /O2;
                /Ob2;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT}
            >
            /MP;
            /GL-;
            /Oi;
            /GF;
            /GT;
            /Oy;
            /Gd;
            /W3;
            /Ot;
            /fp:fast;
            /GR;
            -Wno-invalid-offsetof;
            -Wno-comment;
            -Wno-parentheses;
            -Wno-switch;
            -Wno-empty-body;
            -Wno-address-of-temporary;
            -Wno-constant-logical-operand;
            -Wno-invalid-pp-token;
            -Wno-null-dereference;
            -Wno-int-to-void-pointer-cast;
            -Wno-return-type-c-linkage;
            -Wno-dynamic-class-memaccess;
            -fms-extensions;
            -std=c++17;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /MP;
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /Gy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /Gd;
            /fp:fast;
            /GR;
            /wdcomment;
            /wdparentheses;
            /wdswitch;
            /wdaddress-of-temporary;
            /wdnull-dereference;
            /wdint-to-void-pointer-cast;
            /wdoverloaded-virtual;
            /wdunused-local-typedef;
            /wdmissing-braces;
            /wdempty-body;
            /wdundefined-bool-conversion;
            /wdunused-const-variable;
            /wdunused-function;
            /wdunused-variable;
            /wdmisleading-indentation;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:DebugDX11>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /Zi
            >
            $<$<CONFIG:DebugGL>:
                /MP;
                /std:c++latest;
                /Od;
                /Ob0;
                /RTC1;
                /Oy-;
                /W3;
                /Zi
            >
            $<$<CONFIG:DebugUniversalDX11>:
                /std:c++17;
                /Od;
                /Ob0;
                /Oy-;
                /Zi
            >
            $<$<CONFIG:ReleaseDX11>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseGL>:
                /MP;
                /std:c++latest;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                /W3;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            $<$<CONFIG:ReleaseUniversalDX11>:
                /MP;
                /std:c++17;
                /Ox;
                /Ob2;
                /GT;
                /Oy;
                ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
                /Ot;
                /GS-
            >
            /GL-;
            /Oi;
            /GF;
            /Gd;
            /fp:fast;
            /GR;
            /wd4018;
            /wd4244;
            /wd4312;
            /wd4521;
            /wd4703;
            -fno-delete-null-pointer-checks;
            -Wno-comment;
            -Wno-parentheses;
            -Wno-misleading-indentation;
            -Wno-int-to-void-pointer-cast;
            -Wno-unused-local-typedef;
            -Wno-overloaded-virtual;
            -Wno-switch;
            -Wno-undefined-bool-conversion;
            -Wno-null-dereference;
            -Wno-missing-braces;
            -Wno-invalid-noreturn;
            -Wno-unused-function;
            -Wno-unused-variable;
            -Wno-unused-const-variable;
            -Wno-address-of-temporary;
            -Wno-gnu-folding-constant;
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "1) 64 bit")
        target_link_options(${PROJECT_NAME} PRIVATE
            /NOLOGO
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "2) 32 bit")
        target_link_options(${PROJECT_NAME} PRIVATE
            /NOLOGO
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
        target_link_options(${PROJECT_NAME} PRIVATE
            /NOLOGO
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "4) Web")
        target_link_options(${PROJECT_NAME} PRIVATE
            /NOLOGO
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
        target_link_options(${PROJECT_NAME} PRIVATE
            /NOLOGO
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
        target_link_options(${PROJECT_NAME} PRIVATE
            /NOLOGO
        )
    endif()
endif()

