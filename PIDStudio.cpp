#include "PIDStudio.h"
#include <windows.h>
#include <Commdlg.h>
#include <fstream>
#define RINT(i) read((char*)(&i), 4)
#define RBYTE(b) read((char*)(&b), 1)
#define RLEN(data, len) read((char*)(data), len)
#define WLEN(data, len) write((char*)(&data), len)

void PIDStudio::MenuBarCloseButton() {
	delete this;
}

void PIDStudio::WindowDragStart(){
    menu_dragmode = true;
    menu_drag_x = render_window->getPosition().x-sf::Mouse::getPosition().x;
    menu_drag_y = render_window->getPosition().y-sf::Mouse::getPosition().y;
}

void PIDStudio::WindowDragStop(){
    menu_dragmode = false;
}

void PIDStudio::MenuBarOpenButton()
{
    OPENFILENAME ofn = {0};

    //ZeroMemory(&ofn, sizeof(ofn));

    char *filter = "PID Files (*.PID)\0*.PID\0";
    HWND owner = render_window->getSystemHandle();

    char fileName[MAX_PATH*10] = "";

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH*10;
    ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = "";
    if ( GetOpenFileName(&ofn) )
    {
        if(fileName[ofn.nFileOffset - 1] != '\0')
        {
            OpenedFiles.push_back(new PIDFile(this, fileName));
        }
        else
        {
            char* p = fileName;

            while(*p)
            {
                p += lstrlen(p) + 1;
                if(*p)
                {
                    std::string fp = fileName;
                    fp += '\\';
                    fp += p;
                    OpenedFiles.push_back(new PIDFile(this, fp.c_str()));
                }
            }
        }
    }


}

void PIDFile::Focused(){
    delete this;
}

void PIDStudio::DrawPalette(int sel)
{
    sfg::Image* palview = (sfg::Image*)(&*palette_box->GetChildren()[0]);
    sf::Image IMG; IMG.create(97, 97);
    for(int y=0; y<16; y++)
        for(int x=0; x<16; x++)
            {
                for(int i=0; i<5; i++)
                    for(int j=0; j<5; j++)
                        IMG.setPixel(x*6+i+1, y*6+j+1, OpenedPalettes[sel]->GetColor(y*16+x, false));
            }
    palview->SetImage(IMG);
}

void PIDStudio::DrawPalette(PIDPalette* Pal)
{
    sfg::Image* palview = (sfg::Image*)(&*palette_box->GetChildren()[0]);
    sf::Image IMG; IMG.create(97, 97);
    for(int y=0; y<16; y++)
        for(int x=0; x<16; x++)
            {
                for(int i=0; i<5; i++)
                    for(int j=0; j<5; j++)
                        IMG.setPixel(x*6+i+1, y*6+j+1, Pal->GetColor(y*16+x, false));
            }
    palview->SetImage(IMG);
}

void PIDStudio::PalChange()
{
    sfg::ComboBox* pal_list = (sfg::ComboBox*)(&*palette_box->GetChildren()[1]);
    sfg::CheckButton* palopt1 = (sfg::CheckButton*)(&*palette_box->GetChildren()[2]);
    if(pal_list->GetSelectedItem()==OpenedPalettes.size())
    {
        //add new palette
        OPENFILENAME ofn ;

        ZeroMemory(&ofn, sizeof(ofn));

        char *filter = "Palette Files (*.PAL)\0*.PAL\0";
        HWND owner = render_window->getSystemHandle();

        char fileName[MAX_PATH] = "";

        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = owner;
        ofn.lpstrFilter = filter;
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
        ofn.lpstrDefExt = "";
        if ( GetOpenFileName(&ofn) )
        {
            OpenedPalettes.push_back(new PIDPalette(fileName));
            pal_list->InsertItem(pal_list->GetSelectedItem(), &fileName[strlen(fileName)-10]);
            pal_list->SelectItem(pal_list->GetSelectedItem()-1);
        }
        else {
                pal_list->SelectItem(lastsel);
                return;
        }
    }
    int sel = pal_list->GetSelectedItem();
    DrawPalette(sel);
    if(palopt1->IsActive())
    {
        for(int i=0; i<OpenedFiles.size(); i++)
            OpenedFiles[i]->ConvertTo(OpenedPalettes[sel]);
    }
    else
        for(int i=0; i<OpenedFiles.size(); i++)
            OpenedFiles[i]->SetPalette(OpenedPalettes[sel]);
    lastsel = sel;
}

void PIDFile::SetPalette(PIDPalette* Pal)
{
    sf::Image IMG; IMG.create(m_iW, m_iH);
    for(int x=0; x<m_iW; x++)
        for(int y=0; y<m_iH; y++)
        {
                IMG.setPixel(x, y, Pal->GetColor(m_iData[y*m_iW+x]));
        }
    Image->SetImage(IMG);
    if(m_iFlags & Flag_OwnPalette) { delete m_iPalette; m_iFlags = (FLAGS)((int)m_iFlags-(int)Flag_OwnPalette); }
    m_iPalette = Pal;
}

void PIDFile::ConvertTo(PIDPalette* Pal)
{
    for(int x=0; x<m_iW; x++)
            for(int y=0; y<m_iH; y++)
                m_iData[y*m_iW+x] = Pal->FindColor(m_iPalette->GetColor(m_iData[y*m_iW+x]));
    SetPalette(Pal);
    Modified();
}

void PIDFile::Modified()
{
    if(m_modified) return;

    Window->SetTitle("*"+Window->GetTitle());
    m_modified = true;
}

byte PIDPalette::FindColor(sf::Color color)
{
    if(color.a==0) return 0;
    byte best_id=0;
    int best_diff=256*3;
    for (int i=1; i<256; i++)
    {
        sf::Color tmp = GetColor(i, false);
        int tmpd = abs(tmp.r-color.r)+abs(tmp.g-color.g)+abs(tmp.b-color.b);
        if(tmpd==0)
            return i;
        else if(tmpd<best_diff)
        {
            best_diff = tmpd;
            best_id=i;
        }

    }
    return best_id;
}

PIDPalette::PIDPalette(const char* filepath)
{
    std::ifstream isSource(filepath, std::ios_base::binary | std::ios_base::in);
    if (isSource==NULL) { delete this; }
    else {
        for(int i=0; i<256; i++)
            for(int j=0; j<3; j++)
                isSource.RBYTE(Data[i][j]);
    }
}

PIDPalette::PIDPalette(std::ifstream* isSource)
{
    for(int i=0; i<256; i++)
        for(int j=0; j<3; j++)
            isSource->RBYTE(Data[i][j]);
}

PIDPalette::PIDPalette(bool color)
{
    for(int i=0; i<256; i++)
        for(int j=0; j<3; j++)
            Data[i][j] = !color||j==1?i:0;
}

sf::Color PIDPalette::GetColor(int i, bool ztransparent)
{
    if(i || !ztransparent)
        return sf::Color(Data[i][0], Data[i][1], Data[i][2]);
    else return sf::Color::Transparent;
}

PIDFile::PIDFile(class PIDStudio * engine, const char* filepath)
{
    Engine = engine;
    std::ifstream isSource(filepath, std::ios_base::binary | std::ios_base::in);
    if (isSource==NULL) { delete this; }
    else {
    isSource.RINT(m_iID);
    isSource.RINT(m_iFlags);
    isSource.RINT(m_iW);
    isSource.RINT(m_iH);
    isSource.RLEN(m_iU, 16);

    if( (m_iFlags & Flag_OwnPalette) ){
     isSource.seekg(-768, std::ios_base::end);
     m_iPalette = new PIDPalette(&isSource);
     isSource.seekg(32);
    }

    m_iData = new byte[m_iW*m_iH];
    //ZeroMemory(m_iData, m_iW*m_iH);
    //if( m_hPal != NULL )
     //m_iPixels = new byte[m_iW*m_iH*4];

    int x = 0, y = 0, end = m_iW*m_iH;
    if( (m_iFlags & Flag_Compression) ){
     while(1){
     BYTE num, pixel;
     isSource.RBYTE(num);
     if( num > 128 ){
      int spacing = num - 128;
      for(int iter=0;iter<spacing;iter++){
       m_iData[y*m_iW+x] = 0;
       x++;
       if( x == m_iW )
        break;
      }
      if( x == m_iW ){
       x = 0;
       y++;
      }
      if( y >= m_iH ) break;
      continue;
     }
     for(int i=0;i<num;i++){
      isSource.RBYTE(pixel);
      m_iData[y*m_iW+x] = pixel;
      x++;
      if( x == m_iW ){
       x = 0;
       y++;
      }
      if( y >= m_iH ) break;
     }
     if( y >= m_iH ) break;
     }
    }else{
     while(1){
      byte num, pixel;
      isSource.RBYTE(pixel);
      if( pixel > 192 ){
       num = pixel - 192;
       isSource.RBYTE(pixel);
      }else{
       num = 1;
      }
      for(int i=0;i<num;i++){
       m_iData[y*m_iW+x] = pixel;
       x++;
       if( x == m_iW ){
        x = 0;
        y++;
       }
       if( y >= m_iH ) break;
      }
      if( y >= m_iH ) break;
     }
    }
    isSource.close();
    //std::cout << "x: " << m_iU[0] << "y: " << m_iU[1] << std::endl;
    Window = sfg::Window::Create();
    Window->SetTitle(strrchr(filepath, '\\')+1);
    Window->SetRequisition(sf::Vector2f(m_iW<70?70:m_iW , m_iH));
    Window->SetPosition(sf::Vector2f((float)Engine->render_window->getSize().x/2 - Window->GetRequisition().x/2 , (float)Engine->render_window->getSize().y/2 - Window->GetRequisition().y/2 ));
    Window->GetSignal(sfg::Widget::OnCloseButton).Connect( std::bind( &PIDFile::Close, this ) );
    Image = sfg::Image::Create();
    sfg::ComboBox* pal_list = (sfg::ComboBox*)(&*Engine->palette_box->GetChildren()[1]);
    if(m_iPalette) { SetPalette(m_iPalette); Engine->DrawPalette(m_iPalette); }
    else SetPalette(Engine->OpenedPalettes[pal_list->GetSelectedItem()]);
    Window->Add(Image);
    Engine->desktop.Add(Window);
    Engine->desktop.Update(0);
    Engine->m_sfgui.Display( *Engine->render_window );
    Engine->render_window->display();
    Window->GetSignal(sfg::Window::OnMouseLeftPress ).Connect( [this](){ Engine->ActiveFile = this; } );
    //Window->GetSignal(sfg::Window::OnRightClick ).Connect( std::bind( &PIDFile::Focused, this ) );
    path = filepath;
    }
}

void PIDStudio::MenuBarSaveButton()
{
    for(auto f: OpenedFiles)
        f->Save();
}

void PIDStudio::MenuBarMoveByButton()
{
    MoveByWindow = sfg::Window::Create();
    MoveByWindow->SetTitle("Move Offsets");
    auto m_labela = sfg::Label::Create( "X:" );
    auto m_labelb = sfg::Label::Create( "Y:" );
    auto Spina = sfg::SpinButton::Create(-1000, 1000, 1);
    auto Spinb = sfg::SpinButton::Create(-1000, 1000, 1);
    auto BtnOK = sfg::Button::Create("MOVE");
    auto BtnCA = sfg::Button::Create("CANCEL");
    auto Panel = sfg::Table::Create();
    auto separator = sfg::Separator::Create();
    auto space = sfg::Box::Create();
    Panel->Attach(m_labela, {0, 0, 1, 1}, 0, 2, sf::Vector2f(5, 5) );
    Panel->Attach(Spina, {1, 0, 4, 1}, 3, 3, sf::Vector2f(15, 1) );
    Panel->Attach(space, {0, 1, 5, 1}, 3, 3, sf::Vector2f(5, 5) );
    Panel->Attach(m_labelb, {0, 2, 1, 1}, 0, 2, sf::Vector2f(5, 5) );
    Panel->Attach(Spinb, {1, 2, 4, 1}, 3, 3, sf::Vector2f(15, 1) );
    Panel->Attach(space, {0, 3, 5, 1}, 3, 3, sf::Vector2f(5, 5) );
    Panel->Attach(BtnOK, {0, 4, 2, 1}, 3, 3, sf::Vector2f(7, 2) );
    Panel->Attach(BtnCA, {3, 4, 2, 1}, 3, 3, sf::Vector2f(5, 2) );
    Spina->SetDigits(0);
    Spinb->SetDigits(0);
    MoveByWindow->Add(Panel);
    MoveByWindow->SetRequisition(sf::Vector2f(160, 55));
    MoveByWindow->SetPosition(sf::Vector2f((float)render_window->getSize().x/2 - MoveByWindow->GetRequisition().x/2 , (float)render_window->getSize().y/2 - MoveByWindow->GetRequisition().y/2 ));
    MoveByWindow->SetStyle(sfg::Window::Style::DIALOG | sfg::Window::Style::MOVABLE | sfg::Window::Style::CLOSE | sfg::Window::Style::SHADOW);
    MoveByWindow->GetSignal(sfg::Window::OnCloseButton ).Connect( [this](){ desktop.Remove(MoveByWindow); MoveByWindow = 0; } );
    BtnCA->GetSignal(sfg::Window::OnLeftClick ).Connect( [this](){ desktop.Remove(MoveByWindow); MoveByWindow = 0; } );
    BtnOK->GetSignal(sfg::Window::OnLeftClick ).Connect( [this, Spina, Spinb](){ for(auto f: OpenedFiles) f->MoveOffsets(Spina->GetValue(), Spinb->GetValue()); desktop.Remove(MoveByWindow); MoveByWindow = 0; } );
    desktop.Add(MoveByWindow);
   // MoveByWindow->GrabModal();
    //HINSTANCE hInstance = GetWindowLong(hwnd, GWL_HINSTANCE);
    //WinMain((HINSTANCE)hwnd, render_window->getPosition(), render_window->getSize());
}

void PIDFile::MoveOffsets(int x, int y)
{
    if(!x && !y) return;
    m_iU[0] += x;
    m_iU[1] += y;
    Modified();
}

void PIDFile::Save()
{
    if(!m_modified) return;
    std::ofstream osOut(path, std::ios_base::binary | std::ios_base::out);;
    osOut.WLEN(m_iID, 4);
    osOut.WLEN(m_iFlags, 4);
    osOut.WLEN(m_iW, 4);
    osOut.WLEN(m_iH, 4);
    osOut.WLEN(m_iU, 16);

    int len = m_iW*m_iH;
    if( (m_iFlags & Flag_Compression) ){
     int pos = 0;
     while(1){
      if( m_iData[pos] == 0 ){
       int spacingnum = 1;
       for(int i=pos+1;i<len;i++){
        if( spacingnum >= 127 || m_iData[i] != 0 || i%m_iW == 0 ) break;
        spacingnum++;
       }
       byte b = spacingnum+128;
       osOut.WLEN(b, 1);
       pos += spacingnum;
      }else{
       byte buf[128];
       int buflen = 0;
       for(int i=pos;i<=pos+128&&i<len;i++){
        if( m_iData[i] == 0 ){
         break;
        }else{
         buf[buflen] = m_iData[i];
         buflen++;
        }
       }
       byte blen = buflen;
       osOut.WLEN(blen, 1);
       for(int i=0;i<buflen;i++)
        osOut.WLEN(buf[i], 1);
       pos += buflen;
      }
      if( pos >= len ) break;
     }
    }else{
     int pos = 0;
     while(1){
      int num = 1;
      for(int i=pos+1;i<pos+63&&i<len;i++){
       if( m_iData[i] == m_iData[pos] ){
        num++;
       }else{
        break;
       }
      }
      if( num == 1 ){
       byte b;
       if( m_iData[pos] >= 192 ){
        b = 193;
        osOut.WLEN(b, 1);
        b = m_iData[pos];
        osOut.WLEN(b, 1);
       }else{
        b = m_iData[pos];
        osOut.WLEN(b, 1);
       }
      }else{
       byte b = num+192;
       osOut.WLEN(b, 1);
       b = m_iData[pos];
       osOut.WLEN(b, 1);
      }
      pos += num;
      if( pos >= len ) break;
     }
    }
    osOut.close();
    Window->SetTitle(Window->GetTitle().getData()+1);
    m_modified = false;

}

PIDFile::~PIDFile()
{
    if(Window==0)
    {
      //nie udało się otworzyć
    }
    else
    {
      Window->Remove(Image);
      Engine->desktop.Remove(Window);
      if(m_iFlags & Flag_OwnPalette)
        delete m_iPalette;
      delete[] m_iData;
      auto vec = &Engine->OpenedFiles;
      vec->erase(std::remove(vec->begin(), vec->end(), this), vec->end());
    }
}

PIDStudio::~PIDStudio()
{
    for(auto f : OpenedFiles) delete f;
    render_window->close();
    delete render_window;
    exit(0);
}
