/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

using namespace KPlato;

void RequestResourcesPanelBase::init()
{
    effort = new KPTDurationWidget( groupBox1, "effort" );
    layout5->addMultiCellWidget( effort, 0, 0, 1, 2 );
}
