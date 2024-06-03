set(PROJECT_NAME Engine)

################################################################################
# Source groups
################################################################################
set(H
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Engine.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\" FILES ${H})

set(H__Animation
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Animation/Animation Keys.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Animation/Animation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Animation/Motion.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Animation/Skeleton.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Animation" FILES ${H__Animation})

set(H__Code
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Code Col Line.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Compiler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Environment.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Expression.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Find.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Item.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Macro.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Parser.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Shared.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Source.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Symbol.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE Token.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Code/CE.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Code" FILES ${H__Code})

set(H__Edit
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Edit/Editor Interface.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Edit/Save Changes.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Edit/Undo.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Edit/Version.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Edit/Viewport4.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Edit" FILES ${H__Edit})

set(H__File
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/File/Chunk.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/File/File Text.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/File/File.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/File/IO.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/File/Pak.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/File/Xml.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\File" FILES ${H__File})

set(H__Game
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Area.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Mini Map.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Obj.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Waypoint.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/World.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Game" FILES ${H__Game})

set(H__Game__Objects
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Animatable.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Character.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Destructible.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Door.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Item.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Kinematic.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Obj Decal.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Obj Light Cone.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Obj Light Point.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Obj Particles.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Game/Objects/Static.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Game\\Objects" FILES ${H__Game__Objects})

set(H__Graphics
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Clouds.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Color.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Display Draw.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Display State.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Display.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Effects 2D.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Effects 3D.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Environment.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Fog.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Fur.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Image Atlas.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Image RT.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Image.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Light.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Material.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Particle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Renderer Instancing.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Renderer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Shader Compiler.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Shader Main.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Shader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Sky.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Sun.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Vertex Index Buffer Ex.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Vertex Index Buffer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Graphics/Water.h"
)
source_group("H\\Graphics" FILES ${H__Graphics})

set(H__Gui
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Font.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Gui Object.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Gui Objects.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Gui Skin.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Gui.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Panel Image.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Panel.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Text Style.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Gui" FILES ${H__Gui})

set(H__Gui__Objects
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Button.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/CheckBox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Color Picker.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/ComboBox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Desktop.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Gui Custom.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Gui Image.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/List.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Menu.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/MenuBar.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Progress.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Property.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Region.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/SlideBar.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Slider.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Tabs.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Text.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/TextBox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/TextLine.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Viewport.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Window IO.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Gui/Objects/Window.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Gui\\Objects" FILES ${H__Gui__Objects})

set(H__Input
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Input/Input Buttons.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Input/Input.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Input/Joypad.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Input/Keyboard.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Input/Mouse.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Input/Touch.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Input" FILES ${H__Input})

set(H__Math
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Calculate.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Camera.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Frustum.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Math.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Matrix.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Number.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Orientation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Quaternion.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Random.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shape.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Vector.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Math" FILES ${H__Math})

set(H__Math__Shapes
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Ball.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Box.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Capsule.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Circle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Cone.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Edge.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Plane.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Polygon.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Pyramid.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Quad.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Rectangle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Torus.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Triangle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Math/Shapes/Tube.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Math\\Shapes" FILES ${H__Math__Shapes})

set(H__Memory
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Cache.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Cipher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Counted Pointer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Fixed Array.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Grid.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Hash.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Index.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Map.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem Block.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem Continuous Thread Safe.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem Continuous.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem Extended.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem List.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem Pointer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem Simple.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem Temporary.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Mem Tree.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Memory.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/Node.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Memory" FILES ${H__Memory})

set(H__Memory___
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/_Cache.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/_Grid.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/_Map.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/_Memb.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/_Memc Thread Safe.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/_Memc.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/_Meml.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Memory/_Memx.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Memory\\_" FILES ${H__Memory___})

set(H__Mesh
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Blocks.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Heightmap.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Material Palette.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Base.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Csg.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Cuts.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Group.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Import.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Lod.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Mesh.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Overlay.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Part.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Mesh Render.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Mesh/Voxel.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Mesh" FILES ${H__Mesh})

set(H__Misc
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Application.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Callback.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Character.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Compress.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Cpu.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Date Time.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/DLL.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Enum.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Member.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Misc.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/MP4.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Object.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Param.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Pathfind 2D.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Pathfind.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Printer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Reference.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Sort.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/States.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/String Borrowed.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/String Functions.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/String.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/String8.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Thread.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Timer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Translation.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/UID.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Video.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Misc/Windows.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Misc" FILES ${H__Misc})

set(H__Net
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Advertisement.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Connection.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Download.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Esenthel Store.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Facebook.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Ftp.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Game Analytics.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Internet Cache.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Interpolator.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Patcher.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Socket.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Sql.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Steam.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Store.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Net/Xbox Live.h"
)
source_group("H\\Net" FILES ${H__Net})

set(H__Physics
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Actor.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Aggregate.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Cloth.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Controller.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Destruct Mesh.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Grab.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Joint.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Phys Body.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Phys Material.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Physics.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Ragdoll.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Physics/Vehicle.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Physics" FILES ${H__Physics})

set(H__Platforms__Android
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Platforms/Android.h"
)
source_group("H\\Platforms" FILES ${H__Platforms__Android})
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
set(H__Platforms__Switch
    "${CMAKE_CURRENT_SOURCE_DIR}//NintendoSwitch/Editor/Bin/Engine/Switch.h"
)
endif()
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Platforms" FILES ${H__Platforms__Switch})

set(H__Sound
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Sound/Music.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Sound/Sound Buffer.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Sound/Sound Extra.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Sound/Sound Stream.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/Sound/Sound.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\Sound" FILES ${H__Sound})

set(H___
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_/defines.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_/headers.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_/inline.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_/structs.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Bin/Engine/_/types.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\_" FILES ${H___})

set(H_____OpenGL
"${CMAKE_CURRENT_SOURCE_DIR}/../ThirdPartyLibs/GL/glew.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\..\\Editor\\Bin\\Engine\\_\\OpenGL" FILES ${H_____OpenGL})

set(Source__Animation
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Animation/Animated Skeleton.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Animation/Animation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Animation/Motion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Animation/Skeleton.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Animation" FILES ${Source__Animation})

set(Source__Code
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Code Col Line.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Editor C++.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Editor GUI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Editor IO.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Editor Parser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Editor Sources.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Find.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Item.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Macro.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Parser.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Source C++.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Source Edit.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Source GUI.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Source Preprocessor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Source Suggestions.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Source.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Symbol.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE Token.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/CE.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Code" FILES ${Source__Code})

set(Source__Code__Compile
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/Compile/CE Command.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/Compile/CE Compiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/Compile/CE Environment.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/Compile/CE Expression.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/Compile/CE Func List.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Code/Compile/CE Func List.h"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Code\\Compile" FILES ${Source__Code__Compile})

set(Source__Edit
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Edit/Editor Interface.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Edit/Save Changes.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Edit/Undo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Edit/Version.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Edit/Viewport4.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Edit" FILES ${Source__Edit})

set(Source__File
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/File/Chunk.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/File/File Text.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/File/File.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/File/IO.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/File/Pak.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/File/Xml.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\File" FILES ${Source__File})

set(Source__Game
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Area.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Mini Map.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Obj.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Path 2D Walker.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Waypoint.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/World Draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/World Object.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/World Path 2D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/World Terrain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/World.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Game" FILES ${Source__Game})

set(Source__Game__Objects
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Animatable.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Character Action.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Character Animation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Character Item.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Character Ragdoll.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Character.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Destructible.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Door.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Item.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Kinematic.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Obj Decal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Obj Light Cone.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Obj Light Point.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Obj Particles.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Game/Objects/Static.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Game\\Objects" FILES ${Source__Game__Objects})

set(Source__Graphics
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Clouds.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Color.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Decal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Display Draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Display State.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Display Viewport.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Display.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Effects 2D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Effects 3D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Environment.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Fog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Fur.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Image Atlas.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Image Filter Waifu.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Image IO.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Image Operations.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Image Pixel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Image RT.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Image.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/lights/Light.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Material.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Particle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Renderer Instancing.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Renderer Targets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Renderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Shader Compiler.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Shader Compilers.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Shader Get.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Shader Main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Shader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Sky.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Sun.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Vertex Index Buffer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Water.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Graphics" FILES ${Source__Graphics})

set(Source__Graphics__Import
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/ASTC Compress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/ASTC.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/AVIF Compress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/AVIF.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/BC Compress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/BC.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/BC.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/BMP.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/DDS.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/ETC Compress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/ETC.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/ETC.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/HDR.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/HEIF.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/ICO.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/JPEG XL Compress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/JPEG XL.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/JPEG.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/PNG.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/PSD.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/PVRTC Compress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/PVRTC.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/TGA.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/TIF.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Graphics/Import/WEBP.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Graphics\\Import" FILES ${Source__Graphics__Import})

set(Source__Gui
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Font.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Gui Object.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Gui Objects.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Gui Skin.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Gui.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Panel Image.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Panel.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Text Style.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Gui" FILES ${Source__Gui})

set(Source__Gui__Objects
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Button.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/CheckBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Color Picker.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/ComboBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Desktop.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Gui Custom.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Gui Image.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/List.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Menu.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/MenuBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Progress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Property.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Region.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/SlideBar.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Slider.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Tabs.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Text.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/TextBox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/TextLine.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Viewport.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Window IO.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Gui/Objects/Window.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Gui\\Objects" FILES ${Source__Gui__Objects})

set(Source__Input
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Input/Input.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Input/Joypad.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Input/Keyboard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Input/Mouse.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Input/Oculus Rift.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Input/OpenVR.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Input/Touch.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Input/Virtual Reality.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Input" FILES ${Source__Input})

set(Source__Math
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Calculate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Camera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Frustum.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Math.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Matrix.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Number.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Orientation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Quaternion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Random.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shape.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Vector.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Math" FILES ${Source__Math})

set(Source__Math__Shapes
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Ball.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Box.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Capsule.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Circle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Cone.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Edge.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Plane.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Polygon.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Pyramid.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Quad.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Rectangle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Torus.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Triangle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Math/Shapes/Tube.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Math\\Shapes" FILES ${Source__Math__Shapes})

set(Source__Memory
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Cache.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Cipher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Counted Pointer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Grid.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Hash.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Index.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Map.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Mem Block.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Mem Continuous.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Mem Extended.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Mem List.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Mem Tree.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Memory/Memory.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Memory" FILES ${Source__Memory})

set(Source__Mesh
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Blocks.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Heightmap.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Material Palette.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Base.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Cuts.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Group.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh IO.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Lod.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Optimize.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Overlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Part.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Render.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Mesh Simplify.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Voxel.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Mesh" FILES ${Source__Mesh})

set(Source__Mesh__Import
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Export OBJ.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import 3DS.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import ASE.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import B3D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import BVH.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import DAE.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import FBX.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import MS3D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import OBJ.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import PSK PSA.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/Import/Import XPS.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Mesh\\Import" FILES ${Source__Mesh__Import})

set(Source__Mesh__MeshBase
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Add Remove.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Convert.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Create.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Csg 2D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Csg.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Fix.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Join Split.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Link.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Operations.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Set.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Texturize.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Source/Mesh/MeshBase/Mshb Transform.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Mesh\\MeshBase" FILES ${Source__Mesh__MeshBase})

set(Source__Misc
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Application.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Call Stack.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Callback.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/CMDExecutor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Compress.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Cpu.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Date Time.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/DLL.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Enum.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/InitLogThreadAndDisableCrashDump.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Member.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Misc.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/MP4.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Object.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Param.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Pathfind 2D.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Pathfind.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Printer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Process.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Sort.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/States.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/String.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Thread.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Timer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Translation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Video.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Misc/Windows.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Misc" FILES ${Source__Misc})

set(Source__Net
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/AdMob.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Chartboost.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Connection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Download.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Esenthel Store.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Facebook.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Ftp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Game Analytics.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Internet Cache.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Interpolator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Patcher.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Socket.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Sql.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Steam.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Store.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Net/Xbox Live.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Net" FILES ${Source__Net})

set(Source__Physics
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Actor Create.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Actor.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Aggregate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Cloth.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Controller.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Destruct Mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Grab.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Joint.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Phys Body.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Phys Material.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Physics Draw.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Physics Test.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Physics.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Ragdoll.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Physics/Vehicle.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Physics" FILES ${Source__Physics})

set(Source__Platforms__Android
    "../Editor Data/Code/Android/app/src/main/AndroidManifest.xml"
    "../Editor Data/Code/Android/app/src/main/java/EsenthelActivity.java"
    "../Editor Data/Code/Android/app/src/main/java/Native.java"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Android/Android-C.c"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Android/Android.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Platforms\\Android" FILES ${Source__Platforms__Android})

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
set(Source__Platforms__Mac
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Mac/MyApplication.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Mac/MyApplication.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Mac/MyOpenGLView.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Mac/MyOpenGLView.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Mac/MyWindow.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Mac/MyWindow.mm"
)
endif()
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Platforms\\Mac" FILES ${Source__Platforms__Mac})

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "5) Nintendo Switch")
set(Source__Platforms__Switch
"../NintendoSwitc../Editor/Bin/Engine/Switch.cpp"
)
endif()
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Platforms\\Switch" FILES ${Source__Platforms__Switch})

set(Source__Platforms__Web
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Web/Web.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Platforms\\Web" FILES ${Source__Platforms__Web})

set(Source__Platforms__Windows
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/Windows/Windows Platform.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Platforms\\Windows" FILES ${Source__Platforms__Windows})

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "3) ARM")
set(Source__Platforms__iOS
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/EAGLView.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/EAGLView.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/IAPManager.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/IAPManager.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/iOS.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/iOSAppDelegate.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/iOSAppDelegate.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/MyViewController.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Platforms/iOS/MyViewController.mm"
)
endif()
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Platforms\\iOS" FILES ${Source__Platforms__iOS})

set(Source__Shaders
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/!Header CPU.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/!Header.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/!Set Prec Default.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/!Set Prec High.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/!Set Prec Image.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/!Set Prec Low.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/!Set Prec Struct.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Ambient Occlusion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Ambient Occlusion.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Atmosphere.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Bloom.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Bloom.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Blur.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Cubic.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Cubic.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Depth of Field.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/DX10+ Input Layout.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Effects 2D.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Effects 3D.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/FidelityFX/Fidelity FX.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Fog.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Fog.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Font.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/FXAA.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/FXAA.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Hdr.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Hdr.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Layered Clouds.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Layered Clouds.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Light Apply.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Light Apply.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Light.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Main.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Motion Blur.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Motion Blur.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Outline.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Particles.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Shadow.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Sky.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Sky.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/SMAA.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/SMAA.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Sun.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Temporal.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Temporal.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Video.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Volumetric Clouds.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Volumetric Lights.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Water.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Water.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/World Editor.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Shaders" FILES ${Source__Shaders})

set(Source__Shaders__3D
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Behind.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Blend Light.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Blend.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Bone Highlight.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Deferred.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Early Z.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Emissive.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Forward.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Fur.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Fur.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Mesh Overlay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Mesh Overlay.h"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Overlay.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Position.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Source/Shaders/Set Color.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Shaders\\3D" FILES ${Source__Shaders__3D})

set(Source__Sound
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/App Volume.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/Music.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/Sound Buffer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/Sound Record.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/Sound Resample.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/Sound Stream.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/Sound.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/Spectrometer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Source/Sound/Vorbis Encoder.cpp"
)
source_group("${CMAKE_CURRENT_SOURCE_DIR}\\Source\\Sound" FILES ${Source__Sound})

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

