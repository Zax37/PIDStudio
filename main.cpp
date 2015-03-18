#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#include "PIDStudio.h"
#include "resource.h"
#include <windows.h>
#include <exception>

PIDStudio::PIDStudio(int argc, char* argv[]){
    std::string working_dir(argv[0]);
    working_dir.erase(working_dir.find_last_of('\\')+1);
	render_window = new sf::RenderWindow( sf::VideoMode( SCREEN_WIDTH, SCREEN_HEIGHT ), "PIDStudio", sf::Style::None );
	sf::Image Icon;
	Icon.loadFromMemory(IconPNG, sizeof(IconPNG));
	render_window->setIcon(32, 32, Icon.getPixelsPtr());
	render_window->setFramerateLimit(60); // call it once, after creating the window
    auto MenuBar = sfg::Window::Create();
    desktop.SetProperty( "Window", "BorderColor", sf::Color( 35, 35, 35 ) );
    desktop.SetProperty( "Window", "TitleBackgroundColor", sf::Color( 95, 105, 85 ) );
    desktop.SetProperty( "#workspace", "BackgroundColor", sf::Color( 55, 55, 55 ) );
    desktop.SetProperty( "#menubar", "BackgroundColor", sf::Color( 75, 75, 75 ) );
    sf::Image logo; logo.loadFromFile(working_dir+"logo.png");
	MenuBar->SetStyle(sfg::Window::Style::BACKGROUND);
    auto Logo = sfg::Image::Create(logo);
    auto New = sfg::Button::Create( "New" );
    auto Open = sfg::Button::Create( "Open" );
    auto Save = sfg::Button::Create( "Save" );
    auto Move = sfg::Button::Create( "Move offsets" );
    auto Settings = sfg::Button::Create( "Settings" );
    auto Close = sfg::Button::Create( " X " );
    Open->GetSignal( sfg::Widget::OnLeftClick ).Connect( std::bind( &PIDStudio::MenuBarOpenButton, this ) );
    Save->GetSignal( sfg::Widget::OnLeftClick ).Connect( std::bind( &PIDStudio::MenuBarSaveButton, this ) );
    Move->GetSignal( sfg::Widget::OnLeftClick ).Connect( std::bind( &PIDStudio::MenuBarMoveByButton, this ) );
    Close->GetSignal( sfg::Widget::OnLeftClick ).Connect( std::bind( &PIDStudio::MenuBarCloseButton, this ) );
    auto ButtonBar = sfg::Box::Create( sfg::Box::Orientation::HORIZONTAL, 5.0f );
    ButtonBar->Pack( Logo, true, false );
    ButtonBar->Pack( New, false );
    ButtonBar->Pack( Open, false );
    ButtonBar->Pack( Save, false );
    ButtonBar->Pack( Move, true, false );
    ButtonBar->Pack( Settings, false );
    ButtonBar->Pack( Close, false );
    MenuBar->Add(ButtonBar);
    MenuBar->SetRequisition(sf::Vector2f((float)render_window->getSize().x ,0.f));
    MenuBar->SetPosition(sf::Vector2f((float)render_window->getSize().x - MenuBar->GetRequisition().x ,0.1f));
    MenuBar->GetSignal( sfg::Widget::OnMouseLeftPress ).Connect( std::bind( &PIDStudio::WindowDragStart, this ) );
    MenuBar->GetSignal( sfg::Widget::OnMouseLeftRelease ).Connect( std::bind( &PIDStudio::WindowDragStop, this ) );
    MenuBar->SetId("menubar");
	desktop.Add( MenuBar );
	auto palette = sfg::Window::Create(3);
	palette_box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.0f);
	palette->Add( palette_box );
    sf::Image IMG; IMG.create(97, 97);
	auto palview = sfg::Image::Create(IMG);
	OpenedPalettes.push_back(new PIDPalette());
	OpenedPalettes.push_back(new PIDPalette(true));
	palette_box->Pack( palview );
	DrawPalette(0);
	auto palslist = sfg::ComboBox::Create();
	palslist->AppendItem("Default");
	palslist->AppendItem("Greens");
	palslist->AppendItem("Add...");
	palslist->SelectItem(0);
	palslist->GetSignal( sfg::ComboBox::OnSelect ).Connect( std::bind( &PIDStudio::PalChange, this ) );
	palette_box->Pack( palslist );
	auto palopt1 = sfg::CheckButton::Create("Convert");
	palette_box->Pack( palopt1 );
	palette->SetTitle("Palette");
	palette->SetPosition(sf::Vector2f((float)render_window->getSize().x - palette->GetRequisition().x - 10 , MenuBar->GetRequisition().y+10));
	desktop.Add( palette );

	// We're not using SFML to render anything in this program, so reset OpenGL
	// states. Otherwise we wouldn't see anything.
	render_window->resetGLStates();

	// Main loop!
	sf::Event event;
	sf::Clock clock;
    for(int i=1; i<argc; i++) OpenedFiles.push_back(new PIDFile(this, argv[i]));
	while( render_window->isOpen() ) {
		// Event processing.
		while( render_window->pollEvent( event ) ) {

			// If window is about to be closed, leave program.
			if( event.type == sf::Event::Closed )
                delete this;
            else
                desktop.HandleEvent( event );
		}

        if (menu_dragmode) { render_window->setPosition(sf::Vector2i(menu_drag_x+sf::Mouse::getPosition().x,menu_drag_y+sf::Mouse::getPosition().y)); }

		// Update SFGUI with elapsed seconds since last call.
		desktop.Update( clock.restart().asSeconds() );

		// Rendering.
		render_window->clear(sf::Color(15, 15, 15));
		m_sfgui.Display( *render_window );

		render_window->display();
	}
}

int main(int argc, char* argv[])
{
    try {
    // Try to open the mutex.
    HANDLE hMutex = OpenMutex(
      MUTEX_ALL_ACCESS, 0, "PIDStudio");

    if (!hMutex)
      hMutex =
        CreateMutex(0, 0, "PIDStudio");
    else
    {
        //tutaj sygna³ do istniej¹cego okna
        return 0;
    }

    PIDStudio PIDStudio(argc, argv);
    ReleaseMutex(hMutex);
    }
    catch (std::exception &exception) {
        //Application->ShowException(&exception);
    }
    return 0;
}
