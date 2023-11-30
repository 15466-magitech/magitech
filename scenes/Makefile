.PHONY : all

#n.b. the '-y' sets autoexec scripts to 'on' so that driver expressions will work
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	BLENDER = /Applications/Blender.app/Contents/MacOS/Blender -y
else ifeq ($(UNAME_S),Linux)
	BLENDER = ../../blender-2.93.4-linux-x64/blender -y
else
	BLENDER = blender
endif

EXPORT_MESHES=export-meshes.py
EXPORT_WALKMESHES=export-walkmeshes.py
EXPORT_SCENE=export-scene.py

DIST=../dist

all : \
	$(DIST)/artworld.pnct \
	$(DIST)/artworld.w \
	$(DIST)/artworld.scene \
	$(DIST)/foodworld.pnct \
    $(DIST)/foodworld.w \
    $(DIST)/foodworld.scene \

$(DIST)/artworld.pnct : ../models/artworld_scene_buildout.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':NotWalkMesh '$@'

$(DIST)/artworld.scene : ../models/artworld_scene_buildout.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':NotWalkMesh '$@'

$(DIST)/artworld.w : ../models/artworld_scene_buildout.blend $(EXPORT_WALKMESHES)
	$(BLENDER) --background --python $(EXPORT_WALKMESHES) -- '$<':WalkMeshes '$@'

$(DIST)/foodworld.pnct : ../models/foodworld.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':NotWalkMesh '$@'

$(DIST)/foodworld.scene : ../models/foodworld.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':NotWalkMesh '$@'

$(DIST)/foodworld.w : ../models/foodworld.blend $(EXPORT_WALKMESHES)
	$(BLENDER) --background --python $(EXPORT_WALKMESHES) -- '$<':WalkMeshes '$@'