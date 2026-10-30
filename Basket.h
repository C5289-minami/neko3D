#pragma once
#include "ActorBase.h"
#include "DrawableObject.h"

#include "ResourceKeys.h"

class Basket : public ActorBase
{
public:
	Basket(): ActorBase("Basket")
	{
		model_.SetOwner(*this);
		model_.SetModelKey(ResourceKeys::Model_Basket);
		model_.SetObjectId("basket_model");
		model_.SetObjectName("Basket");
	}

	void Init() override
	{
		Reset();
	}
	void Reset() override
	{
		model_.Load();
	}
	void Update(float) override {}
	void Draw() const override 
	{
		model_.Draw();
	}


	ModelObject& GetModelObject() { return model_; }
	const ModelObject& GetModelObject()const  { return model_; }
private:
	ModelObject model_;
};