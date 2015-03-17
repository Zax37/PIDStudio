#include <SFGUI/SFGUI.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#define byte uint8_t

enum FLAGS {
    Flag_Transparency = 1,
    Flag_VideoMemory = 2,
    Flag_SystemMemory = 4,
    Flag_Mirror = 8,
    Flag_Invert = 16,
    Flag_Compression = 32,
    Flag_Lights = 64,
    Flag_OwnPalette = 128
};

class PIDPalette {
    public:
        PIDPalette(bool color=false);
        PIDPalette(const char* filepath);
        PIDPalette(std::ifstream* isSource);
        sf::Color GetColor(int i, bool ztransparent=true);
        byte FindColor(sf::Color color);
    private:
        byte Data[256][3];
};

class PIDFile {
    friend class PIDStudio;
    public:
        PIDFile(class PIDStudio * engine, const char* filepath);
        ~PIDFile();
        void SetPalette(PIDPalette* Pal);
        void ConvertTo(PIDPalette* Pal);
        void Save();
        void Close(){ delete this; }
    private:
        void Focused();
        sfg::Window::Ptr Window = 0;
        sfg::Image::Ptr Image = 0;
        class PIDStudio * Engine;
        int m_iW, m_iH, m_iID, m_iU[4];
        FLAGS m_iFlags;
        byte* m_iData = 0;
        PIDPalette* m_iPalette = nullptr;
        std::string path;

};

class PIDStudio {
    friend class PIDFile;
	public:
		void MenuBarOpenButton();
		void MenuBarSaveButton();
		void MenuBarCloseButton();
		void WindowDragStart();
		void WindowDragStop();
		void PalChange();
		void MenuBarMoveByButton();
		PIDStudio();
		~PIDStudio();
	private:
	    sfg::Window::Ptr MoveByWindow = 0;
	    int lastsel = 0;
	    void DrawPalette(int sel=0);
	    void DrawPalette(PIDPalette* pal);
		sfg::SFGUI m_sfgui;
        sf::RenderWindow * render_window;
        bool menu_dragmode=false; int menu_drag_x=0, menu_drag_y=0;
        sfg::Desktop desktop;
        std::vector<PIDFile*> OpenedFiles;
        std::vector<PIDPalette*> OpenedPalettes;
        sfg::Box::Ptr palette_box = 0;
        PIDFile* ActiveFile = 0;
};
