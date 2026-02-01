#include "model.h"
#include "animation.h"

int main() {
	/*
	ReadModel("../asset_fbx/player.fbx", "../asset/model/player.mgm");
	ReadModel("../asset_fbx/player_lod1.fbx", "../asset/model/player_lod1.mgm");
	ReadModel("../asset_fbx/player_lod2.fbx", "../asset/model/player_lod2.mgm");
	ReadModel("../asset_fbx/player_lod3.fbx", "../asset/model/player_lod3.mgm");
	ReadModel("../asset_fbx/player_lod4.fbx", "../asset/model/player_lod4.mgm");
	ReadModel("../asset_fbx/plane.fbx", "../asset/model/plane.mgm");
	ReadModel("../asset_fbx/sky.fbx", "../asset/model/sky.mgm");
	ReadModel("../asset_fbx/lamp.fbx", "../asset/model/lamp.mgm");*/
	ReadModel("../asset_fbx/box.fbx", "../asset/model/box.mgm");
	ReadModel("../asset_fbx/sphere.fbx", "../asset/model/sphere.mgm");
	ReadModel("../asset_fbx/crowbar.fbx", "../asset/model/crowbar.mgm");
	ReadModel("../asset_fbx/wooden_axe.fbx", "../asset/model/wooden_axe.mgm");
	ReadModel("../asset_fbx/baseball_bat.fbx", "../asset/model/baseball_bat.mgm");
	ReadModel("../asset_fbx/ground.fbx", "../asset/model/ground.mgm");
	ReadModel("../asset_fbx/shadow.fbx", "../asset/model/shadow.mgm");
	ReadModel("../asset_fbx/alice.fbx", "../asset/model/alice.mgm");
	ReadModel("../asset_fbx/alice_lod1.fbx", "../asset/model/alice_lod1.mgm");
	ReadModel("../asset_fbx/alice_lod2.fbx", "../asset/model/alice_lod2.mgm");
	
	
	
	/*ReadModel("../asset_fbx/test.fbx", "../asset/model/test.mgm");

	ReadAnimation("../asset_fbx/player_idle.fbx", "../asset/animation/player_idle.mga");
	ReadAnimation("../asset_fbx/player_run.fbx", "../asset/animation/player_run.mga");
	ReadAnimation("../asset_fbx/player_attack.fbx", "../asset/animation/player_attack.mga");
	
	ReadAnimation("../asset_fbx/test.fbx", "../asset/animation/test_anim.mga");*/

	ReadAnimation("../asset_fbx/player_impact.fbx", "../asset/animation/player_impact.mga");
	ReadAnimation("../asset_fbx/alice_idle.fbx", "../asset/animation/alice_idle.mga");
	ReadAnimation("../asset_fbx/alice_run.fbx", "../asset/animation/alice_run.mga");
	ReadAnimation("../asset_fbx/alice_attack.fbx", "../asset/animation/alice_attack.mga");
	ReadAnimation("../asset_fbx/alice_impact.fbx", "../asset/animation/alice_impact.mga");
	
	return 0;
}