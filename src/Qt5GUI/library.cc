
/**
 * @file	library.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <api/Runtime.h>	// configuration access
#include <api/Configuration.h>	// config file path
#include "library.h"		// prototypes and GUI library
#include "UI.h"			// library GUI class


/* because these are exported-c functions, we can't have the wrapping namespaces
 * so just use 'using xxx' in each function. */


enum class EUIFlags
{
	Reset,
	SaveLastPosition,
	SaveLastSize
};




int32_t
destroy_interface()
{
	using namespace APP_NAMESPACE;
	using namespace GUI_NAMESPACE;

	int	retval = -1;
	int	x;
	int	y;
	int	w;
	int	h;

	/* do not prompt for confirmation of closure; perform this within the 
	 * main loop itself. Assume we're here only when the window has been, or
	 * is about to be destroyed. */

	/* as g_ui still exists, we still have access to the config document to
	 * make any last-second changes. When reset below, that's it no more! */
#if defined(USING_TCONF)
	tconf::Document*		pdoc = g_ui->ConfigDocument();
	tconf::ConfigurationItem*	ci = pdoc->GetConfigItem("Qt5");
	tconf::ConfigurationItem*	ci_child;

	if ( ci == nullptr )
		goto cleanup;

	ci = ci->Find("main_window");

	if ( g_ui->GetWindowParameters(x, y, w, h) )
	{
		if ( g_ui->ui.flags & EUIFlags::SaveLastPosition )
		{
			ci_child = pdoc->CreateOrGetChildItem(ci, "height");
			ci_child->SetData(std::to_string(h).c_str());
			ci_child = pdoc->CreateOrGetChildItem(ci, "width");
			ci_child->SetData(std::to_string(w).c_str());
		}
		if ( g_ui->ui.flags & EUIFlags::SaveLastSize )
		{
			ci_child = pdoc->CreateOrGetChildItem(ci, "x");
			ci_child->SetData(std::to_string(x).c_str());
			ci_child = pdoc->CreateOrGetChildItem(ci, "y");
			ci_child->SetData(std::to_string(y).c_str());
		}

		// ignore *reporting* any errors saving; the user wants to close
		pdoc->Save();
	}
#endif	// USING_TCONF

	retval = 0;

cleanup:
	// free the global pointer - nothing else should have a link here now!
	g_ui.reset();

	return retval;
}



int32_t
process_interface()
{
	using namespace APP_NAMESPACE;
	using namespace GUI_NAMESPACE;

	g_ui->Run();

	return 0;
}



int32_t
spawn_interface()
{
	using namespace APP_NAMESPACE;
	using namespace GUI_NAMESPACE;

	// move it to the global variable!
	g_ui.reset(new UI);
	g_ui->LoadConfig(runtime.Config()->Path());
	g_ui->CreateDefaultWindows();

	// all created and setup; show the main window and continue
	g_ui->Show();

	return 0;
}
