#ifndef __INCLUDE_SMEX_I_UKRCOOP
#define __INCLUDE_SMEX_I_UKRCOOP

#include <IShareSys.h>

#define SMINTERFACE_UKRCOOP_NAME "IUkrCoop"
#define SMINTERFACE_UKRCOOP_VERSION 3

enum PropTypes
{
	Prop_Sends = 0,
	Prop_Datas
};

typedef enum {
	HINTTEXT = 1,
	CONSOLE,
	CHAT,
	CENTER
}DEST;

namespace SourceMod
{
	class UkrCoop : public SMInterface
	{
	public:
		virtual const char* GetInterfaceName() = 0;
		virtual unsigned int GetInterfaceVersion() = 0;
	public:
		virtual int UkrCoop_GetEntPropEnt(cell_t entity, PropTypes type, const char *prop, int element = 0) = 0;
		virtual bool UkrCoop_SetEntPropEnt(cell_t entity, PropTypes type, const char* prop, cell_t other, int element = 0) = 0;
		virtual int UkrCoop_GetEntProp(cell_t entity, PropTypes type, const char *prop, int size = 4, int element = 0) = 0;
		virtual bool UkrCoop_SetEntProp(cell_t entity, PropTypes type, const char *prop, int value, int size = 4, int element = 0) = 0;

		virtual void UkrCoop_Stargget(int client, int target, Vector *sVector) = 0;
		/**
		 *	*******Повідомлення для ігрока*******
		 *	@brief виводить повідомлення в чаті кліента
		 */
		virtual void UkrCoop_PlayerMsg(int client, DEST type, const char *msg, ...) = 0;
		/**
		 *	*******Лог менеджер*******
		 *	@brief Добавляем повідомлення в файл лога
		 *	і якщо не був созданий файл лога создаем!
		 */
		virtual void UkrCoop_LogMessage(const char *mes, ...) = 0;

		/**
		 *
		 *	******Респавн игрока********
		 *	@brief Возрождае игрока.
		 *
		 */
		virtual void UkrCoop_PlayerRespawn(struct edict_t *pEdict) = 0;
	};
}

#endif