#ifndef EXT_BASE_BOT_H_
#define EXT_BASE_BOT_H_

#include <extplayer.h>
#include <bot/interfaces/playercontrol.h>

class IBotController;
class IBotInterface;

class CBaseBot : public CBaseExtPlayer
{
public:
	CBaseBot(edict_t* edict);
	virtual ~CBaseBot();

	// Called by the manager for all players every server frame.
	// Overriden to call bot functions
	virtual void PlayerThink() override final;

	// true if this is a bot managed by this extension
	virtual bool IsExtensionBot() override { return true; }
	// Pointer to the extension bot class
	virtual CBaseBot* MyBotPointer() override;

	// Reset the bot to it's initial state
	virtual void Reset();
	// Function called at intervals to run the AI 
	virtual void Update();
	// Function called every server frame to run the AI
	virtual void Frame();

	IBotController* GetController() const { return m_controller; }

	void RegisterInterface(IBotInterface* iface);
	void BuildUserCommand();
	inline CBotCmd* GetUserCommand() { return &m_cmd; }
	inline void SetViewAngles(QAngle& angle) { m_viewangles = angle; }
	virtual IPlayerController* GetControlInterface();
private:
	int m_nextupdatetime;
	IBotController* m_controller;
	IBotInterface* m_head; // Interface linked list head. Used to be an std::list but that causes conflicts with the SDK code
	CBotCmd m_cmd; // User command to send
	QAngle m_viewangles; // The bot eye angles
	int m_weaponselect;
	IPlayerController* m_basecontrol; // Base controller interface
};

#endif // !EXT_BASE_BOT_H_
