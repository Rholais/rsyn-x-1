/* Copyright 2014-2017 Rsyn
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef RSYN_MAIN_FRAME_H
#define RSYN_MAIN_FRAME_H

#include "rsyn/gui/frame/base/MainFrameBase.h"
#include "rsyn/gui/canvas/PhysicalCanvasGL.h"
#include "rsyn/gui/canvas/SchematicCanvasGL.h"
#include "rsyn/gui/Redirector.h"
#include "rsyn/util/Json.h"
#include "rsyn/io/parser/script/ScriptReader.h"
#include "rsyn/io/parser/bookshelf/BookshelfParser.h"
#include "rsyn/session/Session.h"
#include "rsyn/util/Stipple.h"

#include <wx/config.h>
#include <wx/filedlg.h>
#include <wx/wfstream.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <vector>
using std::vector;
#include <map>
#include <string>
#include <wx-3.0/wx/propgrid/property.h>

// -----------------------------------------------------------------------------

namespace Rsyn {
class Report;
class Writer;
class RoutingEstimator;
class Timer;
class Graphics;
}

namespace ICCAD15 {
class Infrastructure;
}

class SaveSnapshot;
class AboutDialog;

class MainFrame : public MainFrameBase {
protected:
	CanvasGL *clsCanvasGLPtr = nullptr;
	PhysicalCanvasGL *clsPhysicalCanvasGLPtr = nullptr;
	NewSchematicCanvasGL *clsSchematicCanvasGLPtr = nullptr;
	SchematicCanvasGL *clsSchematicCanvas = nullptr;

private:

	enum View {
		VIEW_PHYSICAL,
		VIEW_SCHEMATIC
	};

	Rsyn::Session clsSession;

	// Physical Design
	Rsyn::PhysicalDesign clsPhysicalDesign;

	// Default Services
	Rsyn::RoutingEstimator * clsRoutingEstimatorPtr = nullptr;
	Rsyn::Timer * clsTimerPtr = nullptr;
	Rsyn::Report * clsReportPtr = nullptr;
	Rsyn::Writer * clsWriterPtr = nullptr;
	Rsyn::Graphics * clsGraphicsPtr = nullptr;

	SaveSnapshot *clsSaveSnapshot = nullptr;
	AboutDialog * clsAboutDialog = nullptr;
	
	wxConfig clsConfig;
	
	static const std::string DEFAULT_STORED_SOLUTION_NAME;
	
	//Update circuit informations
	void updateCircuitInfo();

	// Shows the properties of an instance.
	void updateInstanceProperties(Rsyn::Instance instance, const bool updateOnlyPropertiesAffectedByPlacementChange = false);

	// Shows the properties of a net.
	void updateNetProperties(Rsyn::Net net);

	// Shows the properties of a pin.
	void updatePinProperties(Rsyn::Pin pin);

	// Update stats.
	void UpdateStats(const bool redraw);

	// Open Configure file for ICCAD 2015 Contest.
	void DoRunScript(const string &filename);

	// Save a snapshot.
	void DoSaveSnapshot(const wxString &filename);

	// Do change view.
	void DoChangeView(const View view);

	// Update schematic properties.
	void UpdateSchematicProperties();
	
	// When window is closed, call session Session destructor.
	void OnCloseWindow(wxCloseEvent& event) {
		Destroy();
	} // end method
	
	// Canvas overlay management.
	std::map<std::string, CanvasOverlay *> clsOverlays;
	std::map<wxPGProperty*, int> propToStipple;

	bool startOverlay(const std::string &name, const bool visibility = false);
	
	void startAllOverlays();

	void deleteAllOverlays() {
		for (std::pair<std::string, CanvasOverlay *> p : clsOverlays) {
			delete p.second;
		} // end for		
	} // end method
	
	void configOverlay(const std::string &name, const nlohmann::json &params) {
		auto it = clsOverlays.find(name);
		if (it == clsOverlays.end()) {
			std::cout << "ERROR: Canvas overlay '" << name << "' does not exists.\n";
		} else {
			CanvasOverlay * overlay = it->second;
			overlay->config(params);
		} // end else		
	} // end method

public:
	// Register a process.
	template<typename T>
	static void registerOverlay(const std::string &name, const bool initialVisibility = false) {
		if (!clsRegisteredOverlays) {
			// Try to avoid "static variable initialization fiasco".
			clsRegisteredOverlays = new RegisteredOverlayMap();
		} // end if

		RegisteredOverlayMap &registeredOverlayMap = *clsRegisteredOverlays;

		auto it = registeredOverlayMap.find(name);
		if (it != registeredOverlayMap.end()) {
			std::cout << "ERROR: Process '" << name << "' was already "
					"registered.\n";
			std::exit(1);
		} else {
			//std::cout << "Registering overlay '" << name << "' ...\n";
			registeredOverlayMap[name] = std::make_tuple([]() -> CanvasOverlay *{
				return new T();
			}, initialVisibility);
		} // end else
	} // end method

private:
	// Generic functions used to instantiate overlays.
	typedef std::function<CanvasOverlay *()> OverlayInstantiatonFunction;
	typedef std::unordered_map<std::string, std::tuple<OverlayInstantiatonFunction, bool>> RegisteredOverlayMap;
	static RegisteredOverlayMap *clsRegisteredOverlays;

	// Session events.
	void processGraphicsEventDesignLoaded();
	void processGraphicsEventRefresh();
	void processGraphicsEventUpdateOverlayList();

	// Generate an icon from technology layer pattern.
	wxImage createBitmapFromMask(const unsigned char * mask, wxColor line, wxColor fill);

public:

	MainFrame();
	~MainFrame();

	void openBenchmark(const wxString &filename);

	// Event Handlers.
	virtual void OnQuit(wxCommandEvent &event);

	virtual void OnExecuteCommand(wxCommandEvent& event); 
	virtual void OnExecuteCommandChar(wxKeyEvent& event);
	virtual void OnExecuteCommandKeyDown(wxKeyEvent& event);
	virtual void OnExecuteCommandKeyUp(wxKeyEvent& event);	
	
	virtual void OnRunScript(wxCommandEvent &event);

	virtual void OnLoadPlFile(wxCommandEvent &event);
	virtual void OnLoadColorsFile(wxCommandEvent &event);
	virtual void OnLoadPartFile(wxCommandEvent &event);
	virtual void OnLoadFiedlerFile(wxCommandEvent &event);
	virtual void OnLoadLogicalCoreFile(wxCommandEvent &event);

	virtual void OnSavePlFile(wxCommandEvent &event);
	virtual void OnSaveDEF (wxCommandEvent &event);
	virtual void OnSaveSnapshot(wxCommandEvent &event);

	virtual void OnResetCamera(wxCommandEvent& event);
	virtual void OnZoomIn(wxCommandEvent& event);
	virtual void OnZoomOut(wxCommandEvent& event);

	virtual void OnChangeView(wxCommandEvent& event);
	virtual void OnChangeView(wxChoicebookEvent& event);

	virtual void OnColoringColorful(wxCommandEvent &event);
	virtual void OnColoringRandomBlue(wxCommandEvent &event);
	virtual void OnColoringGray(wxCommandEvent &event);
	virtual void OnColoringLateSlack(wxCommandEvent& event);
	virtual void OnColoringEarlySlack(wxCommandEvent& event);
	virtual void OnColoringCriticality(wxCommandEvent& event);
	virtual void OnColoringCentrality(wxCommandEvent& event);
	virtual void OnColoringRelativity(wxCommandEvent& event);
	
	virtual void OnGenerateColorsFile(wxCommandEvent &event);

	virtual void OnSearch(wxCommandEvent &event);

	virtual void OnCheckCellTimingMode(wxCommandEvent &event);
	virtual void OnOverlayPropertyGridChanged(wxPropertyGridEvent &event);

	virtual void OnKeyDown(wxKeyEvent &event);
	virtual void OnMouseMotion(wxMouseEvent &event);
	virtual void OnLeftUp(wxMouseEvent& event);
	virtual void OnLeftDown(wxMouseEvent& event);
	virtual void OnSelectedCellDrag(wxCommandEvent &event);

	virtual void OnCellSelected(wxCommandEvent &event);
	virtual void OnHoverOverObject(wxCommandEvent &event);

	virtual void OnCheckpoint(wxCommandEvent& event);
	virtual void OnScroll(wxScrollEvent &event);

	virtual void OnUpdateSteinerTrees(wxCommandEvent& event);
	virtual void OnUpdateTiming(wxCommandEvent& event);
	
	virtual void OnLegalize(wxCommandEvent& event);
	virtual void OnLegalizeSelectedCell(wxCommandEvent& event);
	
	virtual void OnRun( wxCommandEvent& event );
		

	// Methods of help menu item
	
	//! @brief Shows a dialog box presenting data about Rsyn.
	virtual void OnAbout(wxCommandEvent& event) override;
	
	
	// Methods for schematic canvas control
	virtual void OnSchematicCellSelected(wxCommandEvent &event);
	virtual void OnSchematicClickView(wxCommandEvent &event) override;
	virtual void OnSchematicNumCriticalPaths(wxCommandEvent & event) override;
}; // end class

// -----------------------------------------------------------------------------

class SaveSnapshot : public SaveSnapshotBase {
private:	
	float ratioWH;
	PhysicalCanvasGL* canvas;
public:
	
	/* Constructor */
	SaveSnapshot(wxWindow* window, PhysicalCanvasGL* canvas) : canvas(canvas),
	SaveSnapshotBase( window ) {
		m_comboBox1->Append( "Nearest (Suggested)" );
		m_comboBox1->Append( "Bilinear" );
		m_comboBox1->Append( "Bicubic" );
		m_comboBox1->Append( "Box average" );
		m_comboBox1->Append( "Normal" );
		m_comboBox1->Append( "High" );
		m_comboBox1->Select( 0 );
	} // end constructor
	
	//--------------------------------------------------------------------------
	
	/* Change the default dimensions and ratio of the image */
	void setDimensions( const int w, const int h ) { 
		m_spinCtrl2->SetValue( w );
		m_spinCtrl3->SetValue( h );
		ratioWH = w / (float) h;
	} // end method
	
	//--------------------------------------------------------------------------
	
	/* Enables the user to save snapshots after the design is loaded */
	void enableWindow() {
		m_spinCtrl2->Enable( true );
		m_spinCtrl3->Enable( true );
		m_textCtrl15->Enable( true );
		m_comboBox1->Enable( true );
		m_checkBox7->Enable( true );
		m_button33->Enable( true );
		m_button34->Enable( true );
		
		if (canvas) {
			m_spinCtrl2->SetValue(canvas->GetSize().x);
			m_spinCtrl3->SetValue(canvas->GetSize().y);
		}
	} // end method
	
	//--------------------------------------------------------------------------
	
	/* Method to keep the aspect ratio */
	void adjustHeight(wxSpinEvent& event) override {
		if( !m_checkBox7->IsChecked() )
			return;
		
		const int width = m_spinCtrl2->GetValue();
		
		 m_spinCtrl3->SetValue( (int) (width / ratioWH) );
	}; // end method
	
	//--------------------------------------------------------------------------

	/* Method to keep the aspect ratio */
	void adjustWidth(wxSpinEvent& event) override {
		if( !m_checkBox7->IsChecked() )
			return;
		
		const int height = m_spinCtrl3->GetValue();
		
		 m_spinCtrl2->SetValue( (int) (height * ratioWH) );
	}; // end method
	
	//--------------------------------------------------------------------------
	
	/* Select the image file */
	void findPath(wxCommandEvent& event) override {
		wxString filename = wxFileSelector(wxT("Save Snapshot"), wxT(""), 
			wxT(""), wxT(""), wxT("*.*"), wxFD_SAVE);
		
		if (!filename.empty())
			m_textCtrl15->SetValue( filename );
	};
	
	//--------------------------------------------------------------------------

	/* Create the snapshot file */
	void saveSnapshot(wxCommandEvent& event) override {
		wxFileName filename( m_textCtrl15->GetValue() );
		
		if( !filename.IsOk() ) {
			wxMessageBox( "Error writting file" );
			return;
		} // end if
		
		if( filename.Exists() && 
			wxMessageBox( "Do you want to overwrite the existing file?", "",  wxYES_NO | wxCENTRE ) != wxYES ) {
			return;
		} // end if
		
		wxImage image;
		canvas->snapshot( image );
		
		const int width = m_spinCtrl2->GetValue();
		const int height = m_spinCtrl3->GetValue();
		
		wxImageResizeQuality resizeQuality;
		
		switch( m_comboBox1->GetSelection() ) {
			case 0:
				resizeQuality = wxImageResizeQuality::wxIMAGE_QUALITY_NEAREST;
				break;
			case 1:
				resizeQuality = wxImageResizeQuality::wxIMAGE_QUALITY_BILINEAR;
				break;
			case 2: 
				resizeQuality = wxImageResizeQuality::wxIMAGE_QUALITY_BICUBIC;
				break;
			case 3:
				resizeQuality = 
					wxImageResizeQuality::wxIMAGE_QUALITY_BOX_AVERAGE;
				break;
			case 4: 
				resizeQuality = wxImageResizeQuality::wxIMAGE_QUALITY_NORMAL;
				break;
			case 5: 
				resizeQuality = wxImageResizeQuality::wxIMAGE_QUALITY_HIGH;
				break;
		}
		
		image.Rescale( width, height, resizeQuality);
		
		image.SaveFile( filename.GetFullPath(), wxBITMAP_TYPE_PNG );
		
		this->Hide();
	};
};

class AboutDialog : public AboutDialogBase {
public:

	/* Constructor */
	AboutDialog(wxWindow* window) : AboutDialogBase(window) {
	}

};

#endif
