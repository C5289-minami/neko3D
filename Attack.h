#pragma once

enum class AttackType
{
    Bang,       // —¼è‚ğ—‚Æ‚·
    TailWhip,   // K”ö
    Clap,       // ”L‚¾‚Ü‚µ
    HairBall,   // –Ñ‹Ê“f‚«
    Bite        // Šš‚İ‚Â‚«
};

class AttackBase
{
    // UŒ‚‚ÌÅ‰‚ÉŒÄ‚Î‚ê‚é
	virtual void Enter() = 0;
	// UŒ‚’†‚É–ˆƒtƒŒ[ƒ€ŒÄ‚Î‚ê‚é
	virtual void Tick(float deltaTime) = 0;
	// UŒ‚‚ªI—¹‚µ‚½‚Æ‚«‚ÉŒÄ‚Î‚ê‚é
	virtual void Exit() = 0;


};