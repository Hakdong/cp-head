#include "stdafx.h"
#include "EventHandlerExample.h"
#include "CommonUtillfunc.h"

CFormViewExample fe;

CEventHandlerExample::CEventHandlerExample()
{
}


CEventHandlerExample::~CEventHandlerExample()
{
}


void CEventHandlerExample::OnInspect(_In_ Algorithms::CRavidImage* pImgInfo, _In_ int nInspectionType, _In_opt_ CRavidImageView* pView , _In_opt_ int nChannel)
{
	do
	{
		OnStartInspection(pView);
	} while (false);
}

bool CEventHandlerExample::OnStartInspection(_In_ CRavidImageView* pView)
{
	bool bReturn = false;
	float fInspTime = 0.0f;

	CRavidImage* pImgInfo = nullptr;

	do
	{
		if (!pView)
			break;

		CRavidImageViewLayer* pLayer0 = pView->GetLayer(0);
		CRavidImageViewLayer* pLayer1 = pView->GetLayer(1);
		CRavidImageViewLayer* pLayer2 = pView->GetLayer(2);

		if (!pLayer0 || !pLayer1 || !pLayer2)
			break;

		pLayer0->Clear();
		pLayer1->Clear();
		pLayer2->Clear();

		pImgInfo = pView->GetImageInfo();

		if (!pImgInfo)
			break;

		CPerformanceCounter	pc;
		
		CRavidImage imgInfoInspect(pImgInfo);

		//Inspction 
		//1. Single threshold
		//DoShapeFInder(imgInfoInspect);
		DoProject2(imgInfoInspect);
		//
		pImgInfo->Copy(imgInfoInspect);

		fInspTime = pc.GetElapsedTime();

		// View 2//////////////////////////////////////
		CRavidImage imgView2;
		imgView2.Copy(imgInfoInspect);

		CRavidImageView* pView2 = dynamic_cast<CRavidImageView*>(CUIManager::FindView(typeid(CRavidImageView), 1));
		CRavidImage* pImgInfo2 = nullptr;
		pView2->Open(L"C.bmp");

		pImgInfo2 = pView2->GetImageInfo();
		pImgInfo2->Copy(imgView2);
		pView2->ZoomFit();
		////////////////////////////////////////////////

		


		pView->Invalidate(false);
		bReturn = true;



	} while (false);

	return bReturn;
}

void CEventHandlerExample::OnTeach(_In_ Algorithms::CRavidImage* pImgInfo, _In_ CTeachingObject* pTeachingObject, _In_opt_ CRavidImageView* pView, _In_opt_ int nChannel)
{
	pView->GetLayer(0)->Clear();

	CRavidRect<double> pRect1 = (CRavidRect<double>*)pTeachingObject->GetAt(0);

	m_rrMeasurement = pRect1;

	pView->GetLayer(0)->DrawShape(pRect1, GREEN, 1);

	pView->Invalidate();
}


void CEventHandlerExample::UpdateResult(std::vector<CMultipleVariable> vctResults)
{
	do
	{
		CRavidGraphView* pRgvHisto = dynamic_cast<CRavidGraphView*>(CUIManager::FindView(typeid(CRavidGraphView), 3));

		if (!pRgvHisto)
			break;

		// draw histogram graph
		pRgvHisto->ClearPlotInfo();
		pRgvHisto->SetXAxisUnit(_T("X (Value)"));
		pRgvHisto->SetYAxisUnit(_T("Y (Index)"));

		std::vector<CRavidPoint<double>> vctData;
		//
		int nCnt = 0;
		double dPlot = 0;
		for (auto& rp : vctResults)
		{
			dPlot = rp.GetAt(1);
			vctData.push_back(CRavidPoint<double>((double)nCnt, dPlot));
			nCnt++;
		}
		DrawGraph(pRgvHisto, vctData, dPlot);

	} while (false);
}
void CEventHandlerExample::DrawGraph(CRavidGraphView* pGraph, const std::vector<CRavidPoint<double>>& vctData, double dblStd)
{
	do
	{
		if (!pGraph)
			break;

		pGraph->ClearPlotInfo();

		if (vctData.empty())
			break;

		CRavidGraphViewPlotInfomation rgvDataStd;
		rgvDataStd.SetPlotType(ERavidGraphViewPlotType_Line);
		rgvDataStd.SetLineColor(LIGHTRED);
		rgvDataStd.SetLineWidth(1);

		bool bCheckNGPrev = vctData[0].y > dblStd || vctData[0].y == 0 ;

		CRavidGraphViewPlotInfomation rgvData;
		rgvData.SetPlotType(ERavidGraphViewPlotType_DotWithLine);
		rgvData.SetLineWidth(1);

		if (!bCheckNGPrev)
		{
			rgvData.SetLineColor(LIME);
			rgvData.SetDotColor(LIME);
		}
		else
		{
			rgvData.SetLineColor(LIGHTRED);
			rgvData.SetDotColor(LIGHTRED);
		}

		try
		{
			for (auto& rp : vctData)
			{
				bool bCheckNG = (rp.y > dblStd || rp.y == 0); // check NG status
				if (bCheckNGPrev != bCheckNG)
				{
					pGraph->AddPlotInfo(&rgvData); // add previous plot info

					rgvData.ClearPlot(); // new plot info
					rgvData.SetPlotType(ERavidGraphViewPlotType_DotWithLine);

					rgvData.SetLineWidth(1);

					if (bCheckNG) // current is NG
					{
						rgvData.SetLineColor(LIGHTRED);
						rgvData.SetDotColor(LIGHTRED);
					}
					else
					{
						rgvData.SetLineColor(LIME);
						rgvData.SetDotColor(LIME);
					}

					bCheckNGPrev = bCheckNG;
				}

				rgvData.AddPlot(rp.x, rp.y);
				rgvDataStd.AddPlot(rp.x, dblStd);
			}

			pGraph->AddPlotInfo(&rgvData);
			pGraph->AddPlotInfo(&rgvDataStd);

			pGraph->DrawGraph();
		}
		catch (const std::exception&)
		{
			pGraph->ClearPlotInfo();
		}
	} while (false);
}

bool CEventHandlerExample::DoProject2(_In_ CRavidImage& imgInfo)
{
	bool bReturn = false;

	CRavidImage imgInfoInspct(imgInfo);
	CRavidImage riImageView0;
	riImageView0.Copy(imgInfoInspct);
	//////////////////////////////////////////// Teaching 영역 CC 사각형 찾기


	CModelInfo* pModel = CModelManager::GetModelInfo(0);

	if (!pModel)
		return -1;

	SType0ModelParam sData;

	if (!pModel->GetData(&sData, sizeof(SType0ModelParam)))
		return -1;


	CMultipleVariable rmvThreshold1;
	rmvThreshold1.AddValue(100.000);
	
	CConnectedComponent cc0;
	cc0.SetSingleThreshold(rmvThreshold1);
	cc0.SetLogicalCondition(ELogicalCondition_Less);
	cc0.SetMultipleChannelCondition(CConnectedComponent::EMultipleChannelCondition_1);

	cc0.Encode(&riImageView0, &m_rrMeasurement);

	double dblFilterValue1 = 300.000;
	cc0.Filter(CConnectedComponent::EFilterItem_BoundingBoxWidth, dblFilterValue1, ELogicalCondition_Less);

	double dblFilterValue2 = 400.000;
	cc0.Filter(CConnectedComponent::EFilterItem_BoundingBoxWidth, dblFilterValue2, ELogicalCondition_Greater);
	
	///////////////////////////////////////// CC 중심점 찾기
	std::vector<CRavidRect<int>> vctRects0;
	vctRects0 = cc0.GetBoundary();
	double Center_x,Center_y,Width,Height =0;

	for (auto& r : vctRects0)
	{
		Center_x = r.GetCenter().x;
		Center_y = r.GetCenter().y;
		Width = r.GetWidth();
		Height = r.GetHeight();
	}
	///////////////////////////////////////////////// 중심원 찾기
	CRavidCircle<double> rcMeasurement;
	rcMeasurement.rpCenter.x = Center_x;
	rcMeasurement.rpCenter.y = Center_y;
	rcMeasurement.radius = 80.0000000;
	rcMeasurement.startDeg = 0.0000000;
	rcMeasurement.deltaDeg = 360.0000000;

	double dblTolerance = 20.000;

	CCircleGauge cg;
	cg.SetTransitionType(CCircleGauge::ETransitionType_BWorWB);
	cg.SetTransitionChoice(CCircleGauge::ETransitionChoice_LargestAmplitude);
	cg.SetThreshold(20);
	cg.SetMinimumAmplitude(10);
	cg.SetThickness(1);
	cg.SetSamplingStep(5.000);
	cg.SetOutLiersThreshold(1.000);
	cg.SetNumOfPass(0);
	cg.SetMeasurementRegion(rcMeasurement, dblTolerance);

	cg.Measure(&riImageView0);

	CRavidCircle<double> rcMeasuredObject;
	std::vector<CRavidPoint<double> > vctMeasuredValidPoints;
	std::vector<CRavidPoint<double> > vctMeasuredInvalidPoints;

	cg.GetMeasuredObject(rcMeasuredObject);
	cg.GetMeasuredValidPoints(vctMeasuredValidPoints);
	cg.GetMeasuredInvalidPoints(vctMeasuredInvalidPoints);



	//////////////////////////////////////////////////////////////////////////////////// ROI 영역 찾기
	double Deflate = sData.minDiff;

	CRavidRect<double> rr1;
	rr1.left= Center_x - 165+ Deflate;
	rr1.top = Center_y - 217+ Deflate;
	rr1.right = Center_x - 25- Deflate;
	rr1.bottom = Center_y -27- Deflate;

	CRavidRect<double> rr2;
	rr2.left = Center_x + 25+ Deflate;
	rr2.top = Center_y - 217+ Deflate;
	rr2.right = Center_x + 165- Deflate;
	rr2.bottom = Center_y - 27- Deflate;

	CRavidRect<double> rr3;
	rr3.left = Center_x - 165+ Deflate;
	rr3.top = Center_y + 28+ Deflate;
	rr3.right = Center_x - 25- Deflate;
	rr3.bottom = Center_y + 217- Deflate;

	CRavidRect<double> rr4;
	rr4.left = Center_x + 25+ Deflate;
	rr4.top = Center_y + 28+ Deflate;
	rr4.right = Center_x + 165- Deflate;
	rr4.bottom = Center_y + 212- Deflate;


	CRavidGeometryArray rp1;
	rr1.GetSubtractionRegion(rcMeasuredObject, &rp1);
	
	CRavidGeometryArray rp2;
	rr2.GetSubtractionRegion(rcMeasuredObject, &rp2);
	CRavidGeometryArray rp3;
	rr3.GetSubtractionRegion(rcMeasuredObject, &rp3);
	CRavidGeometryArray rp4;
	rr4.GetSubtractionRegion(rcMeasuredObject, &rp4);
	///////////////////////////////////////////////////////// ROI 영역 CONNECTED COMPONENT

	CMultipleVariable rmvThreshold;
	rmvThreshold.AddValue(150.000);

	CConnectedComponent cc1;

	cc1.SetSingleThreshold(rmvThreshold);
	cc1.SetLogicalCondition(ELogicalCondition_Less);
	cc1.SetMultipleChannelCondition(CConnectedComponent::EMultipleChannelCondition_1);

	cc1.Encode(&riImageView0, &rp1);

	double dblFilterValue0 = 10.000;
	cc1.Filter(CConnectedComponent::EFilterItem_BoundingBoxWidth, dblFilterValue0, ELogicalCondition_Less);

	CConnectedComponent cc2;

	cc2.SetSingleThreshold(rmvThreshold);
	cc2.SetLogicalCondition(ELogicalCondition_Less);
	cc2.SetMultipleChannelCondition(CConnectedComponent::EMultipleChannelCondition_1);

	cc2.Encode(&riImageView0, &rp2);

	cc2.Filter(CConnectedComponent::EFilterItem_BoundingBoxWidth, dblFilterValue0, ELogicalCondition_Less);
	
	CConnectedComponent cc3;

	cc3.SetSingleThreshold(rmvThreshold);
	cc3.SetLogicalCondition(ELogicalCondition_Less);
	cc3.SetMultipleChannelCondition(CConnectedComponent::EMultipleChannelCondition_1);

	cc3.Encode(&riImageView0, &rp3);

	cc3.Filter(CConnectedComponent::EFilterItem_BoundingBoxWidth, dblFilterValue0, ELogicalCondition_Less);
	
	CConnectedComponent cc4;

	cc4.SetSingleThreshold(rmvThreshold);
	cc4.SetLogicalCondition(ELogicalCondition_Less);
	cc4.SetMultipleChannelCondition(CConnectedComponent::EMultipleChannelCondition_1);

	cc4.Encode(&riImageView0, &rp4);

	cc4.Filter(CConnectedComponent::EFilterItem_BoundingBoxWidth, dblFilterValue0, ELogicalCondition_Less);
	

	do
	{
		CRavidImageView* pView = dynamic_cast<CRavidImageView*>(CUIManager::FindView(typeid(CRavidImageView), 0));
		CRavidImageViewLayer* pLayer0 = pView->GetLayer(0);
		if (!pLayer0)
			break;

		pLayer0->Clear();
		pLayer0->Show(true);

		DoDrawforCC(cc3, 0);
		DoDrawforCC(cc2, 0);
		DoDrawforCC(cc1, 0);
		DoDrawforCC(cc4, 0);

		//pLayer0->DrawShape(rcMeasuredObject, LIGHTGREEN, 2);
		pLayer0->DrawShape(rp1, LIGHTGREEN, 2);
		pLayer0->DrawShape(rp2, LIGHTGREEN, 2);
		pLayer0->DrawShape(rp3, LIGHTGREEN, 2);
		pLayer0->DrawShape(rp4, LIGHTGREEN, 2);
		
		imgInfo.Swap(riImageView0);
		
		bReturn = true;
	} while (false);

	return bReturn;

	
}
