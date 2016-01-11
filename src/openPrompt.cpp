#include "openPrompt.h"
#include <QtWidgets>
//creates the intial case for the prompt and intializes all the variables
// and layout


//Contructor
openPrompt::openPrompt(){
    
    // create buttons and labels
    label = new QLabel(tr("Do you want to load the previous folder loaded?"));
    yes = new QPushButton("&Yes");
    yes->setDefault(true);
    no = new QPushButton ("&No");
    
    // connect buttons to slots
    connect(yes,SIGNAL(clicked()),this,SLOT(s_yesPressed()));
    connect(no, SIGNAL(clicked()), this, SLOT(s_noPressed()));
    
    //set layout
    QHBoxLayout *hbox= new QHBoxLayout;
    QVBoxLayout *vbox= new QVBoxLayout;
    hbox->addWidget(no);
    hbox->addWidget(yes);
    vbox->addWidget(label);
    vbox->addLayout(hbox);
    setLayout(vbox);   
}

// load and close if yes is pressed
void openPrompt::s_yesPressed(){
    emit load();
    close();
}

//close if no is pressed
void openPrompt::s_noPressed(){
    close();
}

