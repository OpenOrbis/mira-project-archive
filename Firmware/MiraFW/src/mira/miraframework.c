#include "miraframework.h"


//
//	Oni-Framework core
//
#include <oni/messaging/messagemanager.h>
#include <oni/init/initparams.h>
#include <oni/rpc/pbserver.h>

//
//	Built-in plugins
//
#include <oni/plugins/pluginmanager.h>
#include <mira/plugins/fileexplorer/fileexplorer_plugin.h>
#include <mira/plugins/logserver/logserver_plugin.h>
#include <mira/plugins/debugger/debugger_plugin.h>
#include <mira/plugins/pluginloader.h>	// Load plugins from file
#include <mira/plugins/orbisutils/orbisutils_plugin.h>
#include <mira/plugins/hen/henplugin.h>

//
//	Utilities
//
#include <oni/utils/logger.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>

#include <oni/utils/hook.h>

#include <mira/utils/injector.h>

//
//	Filesystems
//
#include <mira/fs/overlay/overlayfs.h>

//
//	Free-BSD Specifics
//
#include <sys/eventhandler.h>
#include <sys/proc.h>					// proc
#include <sys/sysent.h>
//#include <string.h> // memmove, memcpy
//#include <stdlib.h> // malloc, free
//#include <assert.h>

#define MIRA_CONFIG_PATH	"/user/mira.ini"

extern uint8_t __noinline mira_installDefaultPlugins();
uint8_t miraframework_installHandlers(struct miraframework_t* framework);
uint8_t miraframework_loadSettings(struct miraframework_t* framework, const char* iniPath);
uint8_t miraframework_installHooks(struct miraframework_t* framework);

//
//	Event hadling stuff
//

// Handle the kernel panics due to suspending
static void mira_onSuspend(struct miraframework_t* framework);
static void mira_onResume(struct miraframework_t* framework);
// Handle execution of new processes for trainers
//static void mira_onExec(struct miraframework_t* framework);

typedef void(*callback_fn)(struct miraframework_t*);

//
//	Start code
//

struct miraframework_t* mira_getFramework()
{
	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!gFramework)
	{
		// We intentionally allocate the larger struct and cast to the basic
		struct miraframework_t* framework = (struct miraframework_t*)kmalloc(sizeof(struct miraframework_t));
		memset(framework, 0, sizeof(*framework));

		if (!framework)
		{
			// Consider this a fatal error
			WriteLog(LL_Error, "could not allocate framework.");

			for (;;)
				__asm__("nop");

			return NULL;
		}

		// Assign the global variable
		gFramework = (struct framework_t*)framework;
	}

	return (struct miraframework_t*)gFramework;
}

#include <capnp_c.h>
#include <addressbook.capnp.h>

#define ASSERT_EQ(x, y) if (x != y) WriteLog(LL_Error, "ASSERT FAILED")

static capn_text chars_to_text(const char* chars)
{
	size_t(*strlen)(const char *str) = kdlsym(strlen);
	return (capn_text) {
		.len = (int)strlen(chars),
			.str = chars,
			.seg = NULL,
	};
}
uint8_t miraframework_initialize(struct miraframework_t* framework)
{
	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!framework)
		return false;
	
	// Zero initialize everything
	memset(framework, 0, sizeof(*framework));

	// Load the settings from file if it exists
	WriteLog(LL_Info, "loading settings from %s", MIRA_CONFIG_PATH);
	if (miraframework_loadSettings(framework, MIRA_CONFIG_PATH))
		WriteLog(LL_Error, "mira configuration file not found, skipping...");

	// Initialize the message manager
	WriteLog(LL_Debug, "initializing message manager");
	framework->framework.messageManager = (struct messagemanager_t*)kmalloc(sizeof(struct messagemanager_t));
	if (!framework->framework.messageManager)
		return false;
	messagemanager_init(framework->framework.messageManager);

	// Initialize the plugin manager
	WriteLog(LL_Debug, "initializing plugin manager");
	framework->framework.pluginManager = (struct pluginmanager_t*)kmalloc(sizeof(struct pluginmanager_t));
	if (!framework->framework.pluginManager)
		return false;
	pluginmanager_init(framework->framework.pluginManager);

	// Initialize the default plugins
	WriteLog(LL_Info, "installing default plugins");
	if (!mira_installDefaultPlugins(framework))
	{
		WriteLog(LL_Error, "could not initialize plugins");
		return false;
	}

	// Register our event handlers
	WriteLog(LL_Info, "installing event handlers");
	if (!miraframework_installHandlers(framework))
	{
		WriteLog(LL_Error, "could not install handlers");
		return false;
	}

	WriteLog(LL_Info, "installing hooks");
	miraframework_installHooks(framework);

	framework->overlayfs = NULL;

	/*WriteLog(LL_Info, "allocating overlayfs");
	
	framework->overlayfs = (struct overlayfs_t*)kmalloc(sizeof(struct overlayfs_t));
	if (!framework->overlayfs)
	{
		WriteLog(LL_Error, "could not allocate overlayfs");
		return false;
	}*/
	//overlayfs_init(framework->overlayfs);

	const char* name = "FirstName LastName";
	const char* email = "firstname@lastname.lan";
	const char* school = "fuck school breh";
	uint8_t buf[4096];
	ssize_t sz = 0;
	{
		struct capn c;
		capn_init_malloc(&c);
		capn_ptr cr = capn_root(&c);
		struct capn_segment* cs = cr.seg;

		struct Person p =
		{
			.id = 17,
			.name = chars_to_text(name),
			.email = chars_to_text(email)
		};
		p.employment_which = Person_employment_school;
		p.employment.school = chars_to_text(school);

		p.phones = new_Person_PhoneNumber_list(cs, 2);
		struct Person_PhoneNumber pn0 = {
		  .number = chars_to_text("123"),
		  .type = Person_PhoneNumber_Type_work,
		};
		set_Person_PhoneNumber(&pn0, p.phones, 0);
		struct Person_PhoneNumber pn1 = {
		  .number = chars_to_text("234"),
		  .type = Person_PhoneNumber_Type_home,
		};
		set_Person_PhoneNumber(&pn1, p.phones, 1);

		Person_ptr pp = new_Person(cs);
		write_Person(&p, pp);
		int setp_ret = capn_setp(capn_root(&c), 0, pp.p);
		ASSERT_EQ(0, setp_ret);
		sz = capn_write_mem(&c, buf, sizeof(buf), 0 /* packed */);
		capn_free(&c);
	}

	{
		// Deserialize `buf[0..sz-1]` to `rp`.
		struct capn rc;
		int init_mem_ret = capn_init_mem(&rc, buf, sz, 0 /* packed */);
		ASSERT_EQ(0, init_mem_ret);
		Person_ptr rroot;
		struct Person rp;
		rroot.p = capn_getp(capn_root(&rc), 0 /* off */, 1 /* resolve */);
		read_Person(&rp, rroot);

		// Assert deserialized values in `rp`
		//EXPECT_EQ(rp.id, (uint32_t)17);
		//EXPECT_CAPN_TEXT_EQ(name, rp.name);
		//EXPECT_CAPN_TEXT_EQ(email, rp.email);

		//EXPECT_EQ(rp.employment_which, Person_employment_school);
		//EXPECT_CAPN_TEXT_EQ(school, rp.employment.school);

		//EXPECT_EQ(2, capn_len(rp.phones));

		struct Person_PhoneNumber rpn0;
		get_Person_PhoneNumber(&rpn0, rp.phones, 0);
		//EXPECT_CAPN_TEXT_EQ("123", rpn0.number);
		//EXPECT_EQ(rpn0.type, Person_PhoneNumber_Type_work);

		struct Person_PhoneNumber rpn1;
		get_Person_PhoneNumber(&rpn1, rp.phones, 1);
		//EXPECT_CAPN_TEXT_EQ("234", rpn1.number);
		//EXPECT_EQ(rpn1.type, Person_PhoneNumber_Type_home);

		capn_free(&rc);
	}

	WriteLog(LL_Info, "miraframework initialized successfully");

	return true;
}

uint8_t miraframework_installHooks(struct miraframework_t* framework)
{
	if (!framework)
		return false;
	
	return true;
}

uint8_t miraframework_installHandlers(struct miraframework_t* framework)
{
	if (!framework)
		return false;

	eventhandler_tag
	(*eventhandler_register)(struct eventhandler_list *list, const char *name,
		void *func, void *arg, int priority) = kdlsym(eventhandler_register);

	// Register our event handlers
	const int32_t prio = 1337;
	EVENTHANDLER_REGISTER(power_suspend, mira_onSuspend, framework, EVENTHANDLER_PRI_LAST + prio);
	EVENTHANDLER_REGISTER(power_resume, mira_onResume, framework, EVENTHANDLER_PRI_LAST + prio);

	return true;
}

uint8_t miraframework_loadSettings(struct miraframework_t* framework, const char* iniPath)
{
	if (!framework || !iniPath)
		return false;

	// TODO: Load the home directory
	framework->framework.homePath = "";

	// TODO: Load the configuration directory
	framework->framework.configPath = "";

	// TODO: Load the download path
	framework->framework.downloadPath = "";

	// TODO: Load the plugins path
	framework->framework.pluginsPath = "";

	return true;
}

static void mira_onSuspend(struct miraframework_t* framework)
{
	if (!framework)
		return;

	// Notify the user that we are suspending
	WriteLog(LL_Warn, "ON SUSPEND %p", framework);

	// Stop the RPC server
	WriteLog(LL_Info, "stopping RPC server.");
	if (!pbserver_shutdown(framework->framework.rpcServer))
		WriteLog(LL_Error, "there was an error stopping the rpc server.");
	
	// Stop the klog server
	WriteLog(LL_Info, "Shutting down plugin manager");
	pluginmanager_shutdown(framework->framework.pluginManager);

	WriteLog(LL_Info, "Disabling hooks");

	WriteLog(LL_Info, "Everything *should* be stable m8");
}

static void mira_onResume(struct miraframework_t* framework)
{
	if (!framework)
		return;

	// TODO: Handle resuming
	WriteLog(LL_Warn, "ON RESUME %p", framework);

	// Initialize the default plugins
	if (!mira_installDefaultPlugins(framework))
		WriteLog(LL_Error, "could not initialize plugins");

	WriteLog(LL_Info, "enabling hooks");
}

uint8_t __noinline mira_installDefaultPlugins(struct miraframework_t* framework)
{
	// Initialize default plugins
	//
	//	Initialize the orbis utilities plugin
	//
	WriteLog(LL_Info, "allocating orbis utilities");
	framework->orbisUtilsPlugin = (struct orbisutils_plugin_t*)kmalloc(sizeof(struct orbisutils_plugin_t));
	if (!framework->orbisUtilsPlugin)
	{
		WriteLog(LL_Error, "error allocating orbis utils plugin");
		return false;
	}
	orbisutils_init(framework->orbisUtilsPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->orbisUtilsPlugin->plugin);

	// Register file transfer plugin
	WriteLog(LL_Info, "allocating file transfer plugin");
	framework->fileTransferPlugin = (struct fileexplorer_plugin_t*)kmalloc(sizeof(struct fileexplorer_plugin_t));
	if (!framework->fileTransferPlugin)
	{
		WriteLog(LL_Error, "error allocating file transfer plugin");
		return false;
	}
	fileexplorer_plugin_init(framework->fileTransferPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->fileTransferPlugin->plugin);

	WriteLog(LL_Info, "allocating logserver");
	framework->logServerPlugin = (struct logserver_plugin_t*)kmalloc(sizeof(struct logserver_plugin_t));
	if (!framework->logServerPlugin)
	{
		WriteLog(LL_Error, "could not allocate log server.");
		return false;
	}
	logserver_init(framework->logServerPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->logServerPlugin->plugin);

	// Initialize the plugin loader to read from file
	WriteLog(LL_Info, "allocating pluginloader");
	framework->pluginLoader = (struct pluginloader_t*)kmalloc(sizeof(struct pluginloader_t));
	if (!framework->pluginLoader)
	{
		WriteLog(LL_Error, "error allocating plugin loader.");
		return false;
	}
	pluginloader_init(framework->pluginLoader);
	pluginloader_loadPlugins(framework->pluginLoader);

	// Debugger
	WriteLog(LL_Debug, "allocating debugger");		
	framework->debuggerPlugin = (struct debugger_plugin_t*)kmalloc(sizeof(struct debugger_plugin_t));
	if (!framework->debuggerPlugin)
	{
		WriteLog(LL_Error, "could not allocate debugger plugin");
		return false;
	}
	debugger_plugin_init(framework->debuggerPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->debuggerPlugin->plugin);

	// Kick off the rpc server thread
	WriteLog(LL_Debug, "allocating rpc server");
	
	if (framework->framework.rpcServer)
	{
		kfree(framework->framework.rpcServer, sizeof(*framework->framework.rpcServer));
		framework->framework.rpcServer = NULL;
	}

	// New pbserver iteration
	framework->framework.rpcServer = (struct pbserver_t*)kmalloc(sizeof(struct pbserver_t));
	if (!framework->framework.rpcServer)
	{
		WriteLog(LL_Error, "could not allocate rpc server.");
		return false;
	}
	pbserver_init(framework->framework.rpcServer);

	// Startup the server, it will kick off the thread
	WriteLog(LL_Info, "starting rpc server");
	if (!pbserver_startup(framework->framework.rpcServer, ONI_RPC_PORT))
	{
		WriteLog(LL_Error, "rpcserver_startup failed");
		return false;
	}

	// TODO: Remove this ifdef once stable, and port to all different platforms
#if ONI_PLATFORM==ONI_PLATFORM_ORBIS_BSD_501
	// Register the hen plugin
	WriteLog(LL_Debug, "allocating hen plugin");
	framework->henPlugin = (struct henplugin_t*)kmalloc(sizeof(struct henplugin_t));
	if (!framework->henPlugin)
	{
		WriteLog(LL_Error, "could not allocate hen plugin");
		return false;
	}
	henplugin_init(framework->henPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->henPlugin->plugin);
#else
	framework->henPlugin = NULL;
#endif

	return true;
}

struct proc* mira_getProc()
{
	struct miraframework_t* fw = mira_getFramework();
	if (!fw)
		return NULL;

	struct initparams_t* params = fw->initParams;
	if (!params)
		return NULL;

	return params->process;
}

struct thread* mira_getMainThread()
{
	struct proc* process = mira_getProc();

	if (process->p_numthreads > 1)
		return process->p_threads.tqh_first;
	else if (process->p_numthreads == 1)
		return process->p_singlethread;
	else
		return NULL;
}