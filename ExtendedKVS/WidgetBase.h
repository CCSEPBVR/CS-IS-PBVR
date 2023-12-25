#ifndef WIDGETBASE_H
#define WIDGETBASE_H
#include <kvs/ClassName>
#include <kvs/RGBAColor>
#include <kvs/EventListener>
#include <kvs/Rectangle>
#include <kvs/Painter>

namespace kvs
{

class ScreenBase;

namespace qt
{

class WidgetBase : public kvs::Rectangle, public kvs::EventListener
{
public:
    enum Anchor
    {
        Fixed, // fixed at the specified position (not anchored)
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
        LeftCenter,
        Center,
        RightCenter
    };

private:
    const WidgetBase* m_parent;
    kvs::ScreenBase* m_screen;
    kvs::Painter m_painter;

    int m_margin;
    kvs::RGBAColor m_background_color;
    kvs::RGBAColor m_background_border_color;
    float m_background_border_width;
    bool m_visible;
    bool m_active;
    Anchor m_anchor;
    Anchor m_corner;

public:
    WidgetBase( kvs::ScreenBase* screen );
    virtual ~WidgetBase();

public:
    const kvs::Font& font() const { return m_painter.font(); }
    int margin() const { return m_margin; }
    const kvs::RGBAColor& backgroundColor() const { return m_background_color; }
    const kvs::RGBAColor& backgroundBorderColor() const { return m_background_border_color; }
    float backgroundBorderWidth() const { return m_background_border_width; }
    bool isVisible() const { return m_visible; }
    bool isActive() const { return m_active; }
    Anchor anchor() const { return m_anchor; }

    void setFont( const kvs::Font& font ) { m_painter.setFont( font ); }
    void setMargin( const int margin ) { m_margin = margin; }
    void setBackgroundColor( const kvs::RGBAColor& color ) { m_background_color = color; }
    void setBackgroundBorderColor( const kvs::RGBAColor& color ) { m_background_border_color = color; }
    void setBackgroundBorderWidth( const float width ) { m_background_border_width = width; }
    void setBackgroundOpacity( const float opacity );
    void setBackgroundBorderOpacity( const float opacity );
    void setVisible( const bool visible = true ) { m_visible = visible; }
    void setActive( const bool active = true ) { m_active = active; }

    // Anchoring to the screen
    void anchorTo( const Anchor anchor ) { m_anchor = anchor; }
    void anchorToTopLeft() { this->anchorTo( TopLeft ); }
    void anchorToTopCenter() { this->anchorTo( TopCenter ); }
    void anchorToTopRight() { this->anchorTo( TopRight ); }
    void anchorToBottomLeft() { this->anchorTo( BottomLeft ); }
    void anchorToBottomCenter() { this->anchorTo( BottomCenter ); }
    void anchorToBottomRight() { this->anchorTo( BottomRight ); }
    void anchorToLeftCenter() { this->anchorTo( LeftCenter ); }
    void anchorToCenter() { this->anchorTo( Center ); }
    void anchorToRightCenter() { this->anchorTo( RightCenter ); }

    // Anchoring to the parent widget
    void anchorTo( const kvs::qt::WidgetBase* parent, const Anchor anchor, const Anchor corner ) { m_parent = parent; m_anchor = anchor; m_corner = corner; }
    void anchorToTopLeft( const kvs::qt::WidgetBase* parent, const Anchor corner ) { this->anchorTo( parent, TopLeft, corner ); }
    void anchorToTopRight( const kvs::qt::WidgetBase* parent, const Anchor corner ) { this->anchorTo( parent, TopRight, corner ); }
    void anchorToBottomLeft( const kvs::qt::WidgetBase* parent, const Anchor corner ) { this->anchorTo( parent, BottomLeft, corner ); }
    void anchorToBottomRight( const kvs::qt::WidgetBase* parent, const Anchor corner ) { this->anchorTo( parent, BottomRight, corner ); }
    void anchorTopLeftTo( const kvs::qt::WidgetBase* parent, const Anchor anchor ) { this->anchorTo( parent, anchor, TopLeft ); }
    void anchorTopLeftToTopRight( const kvs::qt::WidgetBase* parent ) { this->anchorToTopRight( parent, TopLeft ); }
    void anchorTopLeftToBottomRight( const kvs::qt::WidgetBase* parent ) { this->anchorToBottomRight( parent, TopLeft ); }
    void anchorTopLeftToBottomLeft( const kvs::qt::WidgetBase* parent ) { this->anchorToBottomLeft( parent, TopLeft ); }
    void anchorTopRightTo( const kvs::qt::WidgetBase* parent, const Anchor anchor ) { this->anchorTo( parent, anchor, TopRight ); }
    void anchorTopRightToTopLeft( const kvs::qt::WidgetBase* parent ) { this->anchorToTopLeft( parent, TopRight ); }
    void anchorTopRightToBottomLeft( const kvs::qt::WidgetBase* parent ) { this->anchorToBottomLeft( parent, TopRight ); }
    void anchorTopRightToBottomRight( const kvs::qt::WidgetBase* parent ) { this->anchorToBottomRight( parent, TopRight ); }
    void anchorBottomLeftTo( const kvs::qt::WidgetBase* parent, const Anchor anchor ) { this->anchorTo( parent, anchor, BottomLeft ); }
    void anchorBottomLeftToTopLeft( const kvs::qt::WidgetBase* parent ) { this->anchorToTopLeft( parent, BottomLeft ); }
    void anchorBottomLeftToTopRight( const kvs::qt::WidgetBase* parent ) { this->anchorToTopRight( parent, BottomLeft ); }
    void anchorBottomLeftToBottomRight( const kvs::qt::WidgetBase* parent ) { this->anchorToBottomRight( parent, BottomLeft ); }
    void anchorBottomRightTo( const kvs::qt::WidgetBase* parent, const Anchor anchor ) { this->anchorTo( parent, anchor, BottomRight ); }
    void anchorBottomRightToTopLeft( const kvs::qt::WidgetBase* parent ) { this->anchorToTopLeft( parent, BottomRight ); }
    void anchorBottomRightToTopRight( const kvs::qt::WidgetBase* parent ) { this->anchorToTopRight( parent, BottomRight ); }
    void anchorBottomRightToBottomLeft( const kvs::qt::WidgetBase* parent ) { this->anchorToBottomLeft( parent, BottomRight ); }
    void anchorToTop( const kvs::qt::WidgetBase* parent );
    void anchorToBottom( const kvs::qt::WidgetBase* parent );
    void anchorToLeft( const kvs::qt::WidgetBase* parent );
    void anchorToRight( const kvs::qt::WidgetBase* parent );

    void show();
    void hide();

protected:
    kvs::ScreenBase* screen() { return m_screen; }
    kvs::Painter& painter() { return m_painter; }
    kvs::Vec2i anchorPosition();
    virtual void drawBackground();
    virtual int adjustedWidth() { return 0; }
    virtual int adjustedHeight() { return 0; }
    virtual void initializeEvent();
};

} //qt name

} //kvs name

#endif // WIDGETBASE_H
