#include <set>
#include <qwidget.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include "mitkTranferFunction.h"


class QmitkTransferFunctionCanvas : public QWidget {

  Q_OBJECT

  public:

    QmitkTransferFunctionCanvas( QWidget * parent=0, const char * name=0, WFlags f = false );	
      
    // members of QmitkTransferFunctionCanvas follow
    void mousePressEvent( QMouseEvent* mouseEvent );

    virtual void paintEvent( QPaintEvent* e );

    void mouseMoveEvent( QMouseEvent* mouseEvent );
    void mouseReleaseEvent( QMouseEvent* mouseEvent );
    void mouseDoubleClickEvent( QMouseEvent* mouseEvent );
    virtual void PaintElement(QPainter &p,Element* element);
    QPoint HandleToPoint(const Handle &handle) const {
      int ix = (int)(handle.m_Pos.first * width());
      int iy = height() - (int)( handle.m_Pos.second * height());
      return QPoint(ix,iy);
    }
    typedef std::set<Element*> ElementSetType; 	
    ElementSetType m_Elements;
    /*


       void virtual resizeEvent ( QResizeEvent * event );

       void update();*/
    void PaintElementFunction(const ElementSetType &elements, QPainter &p,int penWidth = 0);

    std::pair<Element*,Handle*> GetNearHandle(int x,int y,unsigned int maxSquaredDistance = 6);
    float SquaredDistance(int x, int y, Handle* handle);
    Handle* m_GrabbedHandle;
    Element* m_GrabbedElement;
    std::pair<int,int> m_MoveStart; 
};

class 
