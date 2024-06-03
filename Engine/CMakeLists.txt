set(PROJECT_NAME Engine)

################################################################################
# Source groups
################################################################################
set(H
    "../Editor/Bin/Engine/Engine.h"
)
source_group("H" FILES ${H})

set(H__Animation
    "../Editor/Bin/Engine/Animation/Animation Keys.h"
    "../Editor/Bin/Engine/Animation/Animation.h"
    "../Editor/Bin/Engine/Animation/Motion.h"
    "../Editor/Bin/Engine/Animation/Skeleton.h"
)
source_group("H\\Animation" FILES ${H__Animation})

set(H__Code
    "../Editor/Bin/Engine/Code/CE Code Col Line.h"
    "../Editor/Bin/Engine/Code/CE Compiler.h"
    "../Editor/Bin/Engine/Code/CE Environment.h"
    "../Editor/Bin/Engine/Code/CE Expression.h"
    "../Editor/Bin/Engine/Code/CE Find.h"
    "../Editor/Bin/Engine/Code/CE Item.h"
    "../Editor/Bin/Engine/Code/CE Macro.h"
    "../Editor/Bin/Engine/Code/CE Parser.h"
    "../Editor/Bin/Engine/Code/CE Shared.h"
    "../Editor/Bin/Engine/Code/CE Source.h"
    "../Editor/Bin/Engine/Code/CE Symbol.h"
    "../Editor/Bin/Engine/Code/CE Token.h"
    "../Editor/Bin/Engine/Code/CE.h"
)
source_group("H\\Code" FILES ${H__Code})

set(H__Edit
    "../Editor/Bin/Engine/Edit/Editor Interface.h"
    "../Editor/Bin/Engine/Edit/Save Changes.h"
    "../Editor/Bin/Engine/Edit/Undo.h"
    "../Editor/Bin/Engine/Edit/Version.h"
    "../Editor/Bin/Engine/Edit/Viewport4.h"
)
source_group("H\\Edit" FILES ${H__Edit})

set(H__File
    "../Editor/Bin/Engine/File/Chunk.h"
    "../Editor/Bin/Engine/File/File Text.h"
    "../Editor/Bin/Engine/File/File.h"
    "../Editor/Bin/Engine/File/IO.h"
    "../Editor/Bin/Engine/File/Pak.h"
    "../Editor/Bin/Engine/File/Xml.h"
)
source_group("H\\File" FILES ${H__File})

set(H__Game
    "../Editor/Bin/Engine/Game/Area.h"
    "../Editor/Bin/Engine/Game/Mini Map.h"
    "../Editor/Bin/Engine/Game/Obj.h"
    "../Editor/Bin/Engine/Game/Waypoint.h"
    "../Editor/Bin/Engine/Game/World.h"
)
source_group("H\\Game" FILES ${H__Game})

set(H__Game__Objects
    "../Editor/Bin/Engine/Game/Objects/Animatable.h"
    "../Editor/Bin/Engine/Game/Objects/Character.h"
    "../Editor/Bin/Engine/Game/Objects/Destructible.h"
    "../Editor/Bin/Engine/Game/Objects/Door.h"
    "../Editor/Bin/Engine/Game/Objects/Item.h"
    "../Editor/Bin/Engine/Game/Objects/Kinematic.h"
    "../Editor/Bin/Engine/Game/Objects/Obj Decal.h"
    "../Editor/Bin/Engine/Game/Objects/Obj Light Cone.h"
    "../Editor/Bin/Engine/Game/Objects/Obj Light Point.h"
    "../Editor/Bin/Engine/Game/Objects/Obj Particles.h"
    "../Editor/Bin/Engine/Game/Objects/Static.h"
)
source_group("H\\Game\\Objects" FILES ${H__Game__Objects})

set(H__Graphics
    "../Editor/Bin/Engine/Graphics/Clouds.h"
    "../Editor/Bin/Engine/Graphics/Color.h"
    "../Editor/Bin/Engine/Graphics/Display Draw.h"
    "../Editor/Bin/Engine/Graphics/Display State.h"
    "../Editor/Bin/Engine/Graphics/Display.h"
    "../Editor/Bin/Engine/Graphics/Effects 2D.h"
    "../Editor/Bin/Engine/Graphics/Effects 3D.h"
    "../Editor/Bin/Engine/Graphics/Environment.h"
    "../Editor/Bin/Engine/Graphics/Fog.h"
    "../Editor/Bin/Engine/Graphics/Fur.h"
    "../Editor/Bin/Engine/Graphics/Image Atlas.h"
    "../Editor/Bin/Engine/Graphics/Image RT.h"
    "../Editor/Bin/Engine/Graphics/Image.h"
    "../Editor/Bin/Engine/Graphics/Light.h"
    "../Editor/Bin/Engine/Graphics/Material.h"
    "../Editor/Bin/Engine/Graphics/Particle.h"
    "../Editor/Bin/Engine/Graphics/Renderer Instancing.h"
    "../Editor/Bin/Engine/Graphics/Renderer.h"
    "../Editor/Bin/Engine/Graphics/Shader Compiler.h"
    "../Editor/Bin/Engine/Graphics/Shader Main.h"
    "../Editor/Bin/Engine/Graphics/Shader.h"
    "../Editor/Bin/Engine/Graphics/Sky.h"
    "../Editor/Bin/Engine/Graphics/Sun.h"
    "../Editor/Bin/Engine/Graphics/Vertex Index Buffer Ex.h"
    "../Editor/Bin/Engine/Graphics/Vertex Index Buffer.h"
    "../Editor/Bin/Engine/Graphics/Water.h"
)
source_group("H\\Graphics" FILES ${H__Graphics})

set(H__Gui
    "../Editor/Bin/Engine/Gui/Font.h"
    "../Editor/Bin/Engine/Gui/Gui Object.h"
    "../Editor/Bin/Engine/Gui/Gui Objects.h"
    "../Editor/Bin/Engine/Gui/Gui Skin.h"
    "../Editor/Bin/Engine/Gui/Gui.h"
    "../Editor/Bin/Engine/Gui/Panel Image.h"
    "../Editor/Bin/Engine/Gui/Panel.h"
    "../Editor/Bin/Engine/Gui/Text Style.h"
)
source_group("H\\Gui" FILES ${H__Gui})

set(H__Gui__Objects
    "../Editor/Bin/Engine/Gui/Objects/Button.h"
    "../Editor/Bin/Engine/Gui/Objects/CheckBox.h"
    "../Editor/Bin/Engine/Gui/Objects/Color Picker.h"
    "../Editor/Bin/Engine/Gui/Objects/ComboBox.h"
    "../Editor/Bin/Engine/Gui/Objects/Desktop.h"
    "../Editor/Bin/Engine/Gui/Objects/Gui Custom.h"
    "../Editor/Bin/Engine/Gui/Objects/Gui Image.h"
    "../Editor/Bin/Engine/Gui/Objects/List.h"
    "../Editor/Bin/Engine/Gui/Objects/Menu.h"
    "../Editor/Bin/Engine/Gui/Objects/MenuBar.h"
    "../Editor/Bin/Engine/Gui/Objects/Progress.h"
    "../Editor/Bin/Engine/Gui/Objects/Property.h"
    "../Editor/Bin/Engine/Gui/Objects/Region.h"
    "../Editor/Bin/Engine/Gui/Objects/SlideBar.h"
    "../Editor/Bin/Engine/Gui/Objects/Slider.h"
    "../Editor/Bin/Engine/Gui/Objects/Tabs.h"
    "../Editor/Bin/Engine/Gui/Objects/Text.h"
    "../Editor/Bin/Engine/Gui/Objects/TextBox.h"
    "../Editor/Bin/Engine/Gui/Objects/TextLine.h"
    "../Editor/Bin/Engine/Gui/Objects/Viewport.h"
    "../Editor/Bin/Engine/Gui/Objects/Window IO.h"
    "../Editor/Bin/Engine/Gui/Objects/Window.h"
)
source_group("H\\Gui\\Objects" FILES ${H__Gui__Objects})

set(H__Input
    "../Editor/Bin/Engine/Input/Input Buttons.h"
    "../Editor/Bin/Engine/Input/Input.h"
    "../Editor/Bin/Engine/Input/Joypad.h"
    "../Editor/Bin/Engine/Input/Keyboard.h"
    "../Editor/Bin/Engine/Input/Mouse.h"
    "../Editor/Bin/Engine/Input/Touch.h"
)
source_group("H\\Input" FILES ${H__Input})

set(H__Math
    "../Editor/Bin/Engine/Math/Calculate.h"
    "../Editor/Bin/Engine/Math/Camera.h"
    "../Editor/Bin/Engine/Math/Frustum.h"
    "../Editor/Bin/Engine/Math/Math.h"
    "../Editor/Bin/Engine/Math/Matrix.h"
    "../Editor/Bin/Engine/Math/Number.h"
    "../Editor/Bin/Engine/Math/Orientation.h"
    "../Editor/Bin/Engine/Math/Quaternion.h"
    "../Editor/Bin/Engine/Math/Random.h"
    "../Editor/Bin/Engine/Math/Shape.h"
    "../Editor/Bin/Engine/Math/Vector.h"
)
source_group("H\\Math" FILES ${H__Math})

set(H__Math__Shapes
    "../Editor/Bin/Engine/Math/Shapes/Ball.h"
    "../Editor/Bin/Engine/Math/Shapes/Box.h"
    "../Editor/Bin/Engine/Math/Shapes/Capsule.h"
    "../Editor/Bin/Engine/Math/Shapes/Circle.h"
    "../Editor/Bin/Engine/Math/Shapes/Cone.h"
    "../Editor/Bin/Engine/Math/Shapes/Edge.h"
    "../Editor/Bin/Engine/Math/Shapes/Plane.h"
    "../Editor/Bin/Engine/Math/Shapes/Polygon.h"
    "../Editor/Bin/Engine/Math/Shapes/Pyramid.h"
    "../Editor/Bin/Engine/Math/Shapes/Quad.h"
    "../Editor/Bin/Engine/Math/Shapes/Rectangle.h"
    "../Editor/Bin/Engine/Math/Shapes/Torus.h"
    "../Editor/Bin/Engine/Math/Shapes/Triangle.h"
    "../Editor/Bin/Engine/Math/Shapes/Tube.h"
)
source_group("H\\Math\\Shapes" FILES ${H__Math__Shapes})

set(H__Memory
    "../Editor/Bin/Engine/Memory/Cache.h"
    "../Editor/Bin/Engine/Memory/Cipher.h"
    "../Editor/Bin/Engine/Memory/Counted Pointer.h"
    "../Editor/Bin/Engine/Memory/Fixed Array.h"
    "../Editor/Bin/Engine/Memory/Grid.h"
    "../Editor/Bin/Engine/Memory/Hash.h"
    "../Editor/Bin/Engine/Memory/Index.h"
    "../Editor/Bin/Engine/Memory/Map.h"
    "../Editor/Bin/Engine/Memory/Mem Block.h"
    "../Editor/Bin/Engine/Memory/Mem Continuous Thread Safe.h"
    "../Editor/Bin/Engine/Memory/Mem Continuous.h"
    "../Editor/Bin/Engine/Memory/Mem Extended.h"
    "../Editor/Bin/Engine/Memory/Mem List.h"
    "../Editor/Bin/Engine/Memory/Mem Pointer.h"
    "../Editor/Bin/Engine/Memory/Mem Simple.h"
    "../Editor/Bin/Engine/Memory/Mem Temporary.h"
    "../Editor/Bin/Engine/Memory/Mem Tree.h"
    "../Editor/Bin/Engine/Memory/Memory.h"
    "../Editor/Bin/Engine/Memory/Node.h"
)
source_group("H\\Memory" FILES ${H__Memory})

set(H__Memory___
    "../Editor/Bin/Engine/Memory/_Cache.h"
    "../Editor/Bin/Engine/Memory/_Grid.h"
    "../Editor/Bin/Engine/Memory/_Map.h"
    "../Editor/Bin/Engine/Memory/_Memb.h"
    "../Editor/Bin/Engine/Memory/_Memc Thread Safe.h"
    "../Editor/Bin/Engine/Memory/_Memc.h"
    "../Editor/Bin/Engine/Memory/_Meml.h"
    "../Editor/Bin/Engine/Memory/_Memx.h"
)
source_group("H\\Memory\\_" FILES ${H__Memory___})

set(H__Mesh
    "../Editor/Bin/Engine/Mesh/Blocks.h"
    "../Editor/Bin/Engine/Mesh/Heightmap.h"
    "../Editor/Bin/Engine/Mesh/Material Palette.h"
    "../Editor/Bin/Engine/Mesh/Mesh Base.h"
    "../Editor/Bin/Engine/Mesh/Mesh Csg.h"
    "../Editor/Bin/Engine/Mesh/Mesh Cuts.h"
    "../Editor/Bin/Engine/Mesh/Mesh Group.h"
    "../Editor/Bin/Engine/Mesh/Mesh Import.h"
    "../Editor/Bin/Engine/Mesh/Mesh Lod.h"
    "../Editor/Bin/Engine/Mesh/Mesh Mesh.h"
    "../Editor/Bin/Engine/Mesh/Mesh Overlay.h"
    "../Editor/Bin/Engine/Mesh/Mesh Part.h"
    "../Editor/Bin/Engine/Mesh/Mesh Render.h"
    "../Editor/Bin/Engine/Mesh/Voxel.h"
)
source_group("H\\Mesh" FILES ${H__Mesh})

set(H__Misc
    "../Editor/Bin/Engine/Misc/Application.h"
    "../Editor/Bin/Engine/Misc/Callback.h"
    "../Editor/Bin/Engine/Misc/Character.h"
    "../Editor/Bin/Engine/Misc/Compress.h"
    "../Editor/Bin/Engine/Misc/Cpu.h"
    "../Editor/Bin/Engine/Misc/Date Time.h"
    "../Editor/Bin/Engine/Misc/DLL.h"
    "../Editor/Bin/Engine/Misc/Enum.h"
    "../Editor/Bin/Engine/Misc/Member.h"
    "../Editor/Bin/Engine/Misc/Misc.h"
    "../Editor/Bin/Engine/Misc/MP4.h"
    "../Editor/Bin/Engine/Misc/Object.h"
    "../Editor/Bin/Engine/Misc/Param.h"
    "../Editor/Bin/Engine/Misc/Pathfind 2D.h"
    "../Editor/Bin/Engine/Misc/Pathfind.h"
    "../Editor/Bin/Engine/Misc/Printer.h"
    "../Editor/Bin/Engine/Misc/Reference.h"
    "../Editor/Bin/Engine/Misc/Sort.h"
    "../Editor/Bin/Engine/Misc/States.h"
    "../Editor/Bin/Engine/Misc/String Borrowed.h"
    "../Editor/Bin/Engine/Misc/String Functions.h"
    "../Editor/Bin/Engine/Misc/String.h"
    "../Editor/Bin/Engine/Misc/String8.h"
    "../Editor/Bin/Engine/Misc/Thread.h"
    "../Editor/Bin/Engine/Misc/Timer.h"
    "../Editor/Bin/Engine/Misc/Translation.h"
    "../Editor/Bin/Engine/Misc/UID.h"
    "../Editor/Bin/Engine/Misc/Video.h"
    "../Editor/Bin/Engine/Misc/Windows.h"
)
source_group("H\\Misc" FILES ${H__Misc})

set(H__Net
    "../Editor/Bin/Engine/Net/Advertisement.h"
    "../Editor/Bin/Engine/Net/Connection.h"
    "../Editor/Bin/Engine/Net/Download.h"
    "../Editor/Bin/Engine/Net/Esenthel Store.h"
    "../Editor/Bin/Engine/Net/Facebook.h"
    "../Editor/Bin/Engine/Net/Ftp.h"
    "../Editor/Bin/Engine/Net/Game Analytics.h"
    "../Editor/Bin/Engine/Net/Internet Cache.h"
    "../Editor/Bin/Engine/Net/Interpolator.h"
    "../Editor/Bin/Engine/Net/Patcher.h"
    "../Editor/Bin/Engine/Net/Socket.h"
    "../Editor/Bin/Engine/Net/Sql.h"
    "../Editor/Bin/Engine/Net/Steam.h"
    "../Editor/Bin/Engine/Net/Store.h"
    "../Editor/Bin/Engine/Net/Xbox Live.h"
)
source_group("H\\Net" FILES ${H__Net})

set(H__Physics
    "../Editor/Bin/Engine/Physics/Actor.h"
    "../Editor/Bin/Engine/Physics/Aggregate.h"
    "../Editor/Bin/Engine/Physics/Cloth.h"
    "../Editor/Bin/Engine/Physics/Controller.h"
    "../Editor/Bin/Engine/Physics/Destruct Mesh.h"
    "../Editor/Bin/Engine/Physics/Grab.h"
    "../Editor/Bin/Engine/Physics/Joint.h"
    "../Editor/Bin/Engine/Physics/Phys Body.h"
    "../Editor/Bin/Engine/Physics/Phys Material.h"
    "../Editor/Bin/Engine/Physics/Physics.h"
    "../Editor/Bin/Engine/Physics/Ragdoll.h"
    "../Editor/Bin/Engine/Physics/Vehicle.h"
)
source_group("H\\Physics" FILES ${H__Physics})

set(H__Platforms__Android
    "../Editor/Bin/Engine/Platforms/Android.h"
)
source_group("H\\Platforms" FILES ${H__Platforms__Android})
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
set(H__Platforms__Switch
    "../NintendoSwitc../Editor/Bin/Engine/Switch.h"
)
endif()
source_group("H\\Platforms" FILES ${H__Platforms__Switch})

set(H__Sound
    "../Editor/Bin/Engine/Sound/Music.h"
    "../Editor/Bin/Engine/Sound/Sound Buffer.h"
    "../Editor/Bin/Engine/Sound/Sound Extra.h"
    "../Editor/Bin/Engine/Sound/Sound Stream.h"
    "../Editor/Bin/Engine/Sound/Sound.h"
)
source_group("H\\Sound" FILES ${H__Sound})

set(H___
    "../Editor/Bin/Engine/_/defines.h"
    "../Editor/Bin/Engine/_/headers.h"
    "../Editor/Bin/Engine/_/inline.h"
    "../Editor/Bin/Engine/_/structs.h"
    "../Editor/Bin/Engine/_/types.h"
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
    "../Engine/Source/Math/Calculate.cpp"
    "../Engine/Source/Math/Camera.cpp"
    "../Engine/Source/Math/Frustum.cpp"
    "../Engine/Source/Math/Math.cpp"
    "../Engine/Source/Math/Matrix.cpp"
    "../Engine/Source/Math/Number.cpp"
    "../Engine/Source/Math/Orientation.cpp"
    "../Engine/Source/Math/Quaternion.cpp"
    "../Engine/Source/Math/Random.cpp"
    "../Engine/Source/Math/Shape.cpp"
    "../Engine/Source/Math/Vector.cpp"
)
source_group("Source\\Math" FILES ${Source__Math})

set(Source__Math__Shapes
    "../Engine/Source/Math/Shapes/Ball.cpp"
    "../Engine/Source/Math/Shapes/Box.cpp"
    "../Engine/Source/Math/Shapes/Capsule.cpp"
    "../Engine/Source/Math/Shapes/Circle.cpp"
    "../Engine/Source/Math/Shapes/Cone.cpp"
    "../Engine/Source/Math/Shapes/Edge.cpp"
    "../Engine/Source/Math/Shapes/Plane.cpp"
    "../Engine/Source/Math/Shapes/Polygon.cpp"
    "../Engine/Source/Math/Shapes/Pyramid.cpp"
    "../Engine/Source/Math/Shapes/Quad.cpp"
    "../Engine/Source/Math/Shapes/Rectangle.cpp"
    "../Engine/Source/Math/Shapes/Torus.cpp"
    "../Engine/Source/Math/Shapes/Triangle.cpp"
    "../Engine/Source/Math/Shapes/Tube.cpp"
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
    "../Engine/Source/Mesh/Blocks.cpp"
    "../Engine/Source/Mesh/Heightmap.cpp"
    "../Engine/Source/Mesh/Material Palette.cpp"
    "../Engine/Source/Mesh/Mesh Base.cpp"
    "../Engine/Source/Mesh/Mesh Cuts.cpp"
    "../Engine/Source/Mesh/Mesh Draw.cpp"
    "../Engine/Source/Mesh/Mesh Group.cpp"
    "../Engine/Source/Mesh/Mesh IO.cpp"
    "../Engine/Source/Mesh/Mesh Lod.cpp"
    "../Engine/Source/Mesh/Mesh Mesh.cpp"
    "../Engine/Source/Mesh/Mesh Optimize.cpp"
    "../Engine/Source/Mesh/Mesh Overlay.cpp"
    "../Engine/Source/Mesh/Mesh Part.cpp"
    "../Engine/Source/Mesh/Mesh Render.cpp"
    "../Engine/Source/Mesh/Mesh Simplify.cpp"
    "../Engine/Source/Mesh/Voxel.cpp"
)
source_group("Source\\Mesh" FILES ${Source__Mesh})

set(Source__Mesh__Import
    "../Engine/Source/Mesh/Import/Export OBJ.cpp"
    "../Engine/Source/Mesh/Import/Import 3DS.cpp"
    "../Engine/Source/Mesh/Import/Import ASE.cpp"
    "../Engine/Source/Mesh/Import/Import B3D.cpp"
    "../Engine/Source/Mesh/Import/Import BVH.cpp"
    "../Engine/Source/Mesh/Import/Import DAE.cpp"
    "../Engine/Source/Mesh/Import/Import FBX.cpp"
    "../Engine/Source/Mesh/Import/Import MS3D.cpp"
    "../Engine/Source/Mesh/Import/Import OBJ.cpp"
    "../Engine/Source/Mesh/Import/Import PSK PSA.cpp"
    "../Engine/Source/Mesh/Import/Import XPS.cpp"
)
source_group("Source\\Mesh\\Import" FILES ${Source__Mesh__Import})

set(Source__Mesh__MeshBase
    "../Engine/Source/Mesh/MeshBase/Mshb Add Remove.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Convert.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Create.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Csg 2D.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Csg.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Fix.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Join Split.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Link.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Operations.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Set.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Texturize.cpp"
    "../Engine/Source/Mesh/MeshBase/Mshb Transform.cpp"
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
#    "../NintendoSwitc../Editor/Bin/Engine/Switch.cpp"
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
    ${H__Platforms__Android}
    ${H__Platforms__Switch}
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
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
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
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
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
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
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
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
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
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_;"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_;"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_;"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_;"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_;"
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
        "${CMAKE_CURRENT_SOURCE_DIR}/../NintendoSwitc../Editor/Bin/Engine/ThirdPartyLibs/PhysX/physx/source/foundation/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../NintendoSwitc../Editor/Bin/Engine/ThirdPartyLibs/PhysX/pxshared/include;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Imath;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Iex;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/lib/IlmBase/Half;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/OpenEXR/config;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/OpenEXR/Windows/IlmBase/config"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "6) Android")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_;"
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
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
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
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
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
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
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

