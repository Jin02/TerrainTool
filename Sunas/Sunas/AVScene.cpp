#include "AVScene.h"
#include "AVDirector.h"

AVScene::AVScene(void* data)
{
}

AVScene::~AVScene(void)
{
}

void AVScene::ChangeScene(AVScene* scene, bool isDelete)
{
	if(isDelete) delete AVDirector::GetDiector()->GetScene();
	AVDirector::GetDiector()->SetScene(scene);
}