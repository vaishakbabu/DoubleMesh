#include "DoubleMesh.h"

#include<maya/MString.h>
#include<maya/MItDag.h>
#include<maya/MBoundingBox.h>
#include<maya/MMatrix.h>
#include<maya/MPointArray.h>
#include<vector>
#include<algorithm>



MStatus checkDuplicateMesh::doIt(const MArgList& args)
{
	//get the active selection list
	MSelectionList mSelectionList;
	MGlobal::getActiveSelectionList(mSelectionList);
	MGlobal::clearSelectionList();
	MItDag dagIterator(MItDag::kDepthFirst, MFn::kMesh);

	//if nothing is selected
	if (mSelectionList.length() == 0)
	{
		//add all mesh type from scene to selectionList
		while (!dagIterator.isDone())
		{
			//avoiding intermediate objects
			MFnDagNode mfnDagNode(dagIterator.currentItem());
			if (!mfnDagNode.isIntermediateObject())
			{
				mSelectionList.add(dagIterator.fullPathName());
			}
			dagIterator.next();
		}

		//if mesh found in scene
		if (mSelectionList.length() != 0)
		{
			iterateFn(mSelectionList);
		}
		else
		{
			MGlobal::displayInfo("No Mesh present in the scene !!");
		}
	}
	else //if user has selected a group of mesh
	{
		MDagPath selDagPath;
		MSelectionList childSelList;

		for (unsigned int i = 0; i < mSelectionList.length(); i++)
		{
			mSelectionList.getDagPath(i, selDagPath);

			//select all children shapes
			dagIterator.reset(selDagPath, MItDag::kDepthFirst, MFn::kMesh);
			while (!dagIterator.isDone())
			{
				//avoiding intermediate objects
				MFnDagNode mfnDagNode(dagIterator.currentItem());
				if (!mfnDagNode.isIntermediateObject())
				{
					childSelList.add(dagIterator.fullPathName());
				}

				dagIterator.next();
			}
		}

		iterateFn(childSelList);

	}
	

	return MS::kSuccess;
}

///////////////////////////////////////////////////////////////////////////////////
/// This function iterates through the selectionList passed to it.		///
/// Compares every dagPath with everyother by feeding them to checkDoubleMesh() ///
///////////////////////////////////////////////////////////////////////////////////

void checkDuplicateMesh::iterateFn(MSelectionList& selList)
{
	int value = 0;

	//iterate through selection list and pass pairs to checkDoubleMesh() function

	for (unsigned int i = 0; i < selList.length(); i++)
	{
		for (unsigned int j = i + 1; i < selList.length(); j++)
		{
			MDagPath dagPath1, dagPath2;

			selList.getDagPath(i, dagPath1);
			selList.getDagPath(j, dagPath2);

			//if both dagpaths points to mesh type, check for doubleMesh
			if (dagPath1.apiType() == MFn::kMesh && dagPath2.apiType() == MFn::kMesh)
			{
				value = checkDoubleMesh(dagPath1, dagPath2);
			}

			if (value=1)
			{
				//select both the transform nodes
				MStringArray dagPathStringArray1;
				MStringArray dagPathStringArray2;
				dagPath1.fullPathName().split('|', dagPathStringArray1);
				dagPath2.fullPathName().split('|', dagPathStringArray2);
				MGlobal::selectByName(dagPathStringArray1[dagPathStringArray1.length() - 2]);
				MGlobal::selectByName(dagPathStringArray2[dagPathStringArray2.length() - 2]);
			}


		}

	}

	MSelectionList finalSelList;
	MGlobal::getActiveSelectionList(finalSelList);

	if (finalSelList.length() == 0)
	{
		MGlobal::displayInfo("No Duplicates found");
	}

}

/////////////////////////////////////////////////////////////////////////////////
/// This function creates two bounding boxes for the dagPaths passed.	      ///
/// Checks if the bounding boxes are at exact same position with a tolerance  ///
/////////////////////////////////////////////////////////////////////////////////

int checkDuplicateMesh::checkDoubleMesh(MDagPath& dagPath1, MDagPath& dagPath2)
{
	double tol = 0.01; //tolerance value
	
	///////////////////////////////////
	// Make the first worldspace bbox
	///////////////////////////////////
		
	MFnDagNode dagFn1(dagPath1);

	MBoundingBox bbox1 = dagFn1.boundingBox();

	//get worldspace points
	MPoint min1 = bbox1.min() * dagPath1.exclusiveMatrix();
	MPoint max1 = bbox1.max() * dagPath1.exclusiveMatrix();

	//make a worldspace bounding box
	MBoundingBox bBoxWorld1(min1, max1);

	/////////////////////////////////////
	// Make the second worldspace bbox
	/////////////////////////////////////

	MFnDagNode dagFn2(dagPath2);

	MBoundingBox bbox2 = dagFn2.boundingBox();

	//get worldspace points
	MPoint min2 = bbox2.min() * dagPath2.exclusiveMatrix();
	MPoint max2 = bbox2.max() * dagPath2.exclusiveMatrix();

	//make a worldspace bounding box
	MBoundingBox bBoxWorld2(min2, max2);

	
	//compare the bounding boxes to identify double mesh cases

	if (min1.isEquivalent(min2, tol) && max1.isEquivalent(max2, tol))
	{
		MGlobal::displayInfo("Duplicate Mesh detected");
		MGlobal::displayInfo(dagPath1.fullPathName());
		MGlobal::displayInfo(dagPath2.fullPathName());
		return 1;
	}
	else
		return 0;

}

MStatus checkDuplicateFace::doIt(const MArgList& args)
{
	//get the active selectionList
	MSelectionList mSelectionList;
	MGlobal::getActiveSelectionList(mSelectionList);
	MGlobal::clearSelectionList();

	MItDag dagIterator(MItDag::kDepthFirst, MFn::kMesh);

	//if nothing is selected
	if (mSelectionList.length()==0)
	{

		//add all mesh type from scene to selectionList
		while (!dagIterator.isDone())
		{
			mSelectionList.add(dagIterator.fullPathName());
			dagIterator.next();
		}

		//if Mesh is found in scene
		if (mSelectionList.length() != 0)
		{
			iterateFn(mSelectionList);
		}
		else
		{
			MGlobal::displayInfo("No mesh present in the scene");
		}
	}
	else    //if user has selected a group of mesh
	{
		MDagPath selDagPath;
		MSelectionList childSelList;

		for (unsigned int i = 0; i < mSelectionList.length(); i++)
		{
			mSelectionList.getDagPath(i, selDagPath);

			//select all children shapes
			dagIterator.reset(selDagPath, MItDag::kDepthFirst, MFn::kMesh);
			while (!dagIterator.isDone())
			{
				childSelList.add(dagIterator.fullPathName());
				dagIterator.next();
			}
		}

		iterateFn(childSelList);
	}
	return MS::kSuccess;
}

////////////////////////////////////////////////////
/// This function iterates through the faces and ///
/// feeds them to checkPolygonPoints()			 ///
////////////////////////////////////////////////////

void checkDuplicateFace::iterateFn(MSelectionList& selList)
{

	for (unsigned int i = 0; i < selList.length(); i++)
	{
		MDagPath dag_path;
		selList.getDagPath(i, dag_path);
		MFnMesh meshFn(dag_path);

		int polycount = meshFn.numPolygons();

		std::vector<int> duplicate_faces;

		for (int i = 0; i < polycount; i++)
		{
			for (int j = i + 1; j < polycount; j++)
			{
				bool condition = checkPolygonPoints(meshFn, i, j);

				if (condition = true)
				{
					duplicate_faces.push_back(i);
				}
			}
		}

		if (duplicate_faces.size() > 0)
		{
			MGlobal::displayInfo("Duplicate Faces detected in " + dag_path.fullPathName()); \

				//add each of the duplicate faces to the current selection list

				for (int i = 0; i < duplicate_faces.size(); i++)
				{
					MString face = dag_path.fullPathName() + ".f[" + duplicate_faces[i] + "]";
					MGlobal::selectByName(face);
				}
		}

	}

	//if selection list is empty - display 'no duplicate faces found'
	MSelectionList finalSelectionList;

	MGlobal::getActiveSelectionList(finalSelectionList);
	if (finalSelectionList.length() == 0)
	{
		MGlobal::displayInfo("No duplicate face found!");
	}

}

////////////////////////////////////////////////////////////////
/// This function checks the distance between vertex points  ///
/// checks if points are close beyond a threshold	     ///
////////////////////////////////////////////////////////////////


bool checkDuplicateFace::checkPolygonPoints(MFnMesh& meshFn, int a_id, int b_id)
{
	double threshold = 0.001;    //threshold value

	MPointArray a_allPoints;
	MIntArray a_all_vert_ids;
	meshFn.getPolygonVertices(a_id, a_all_vert_ids);
	a_allPoints.setLength(a_all_vert_ids.length());

	for (unsigned int i = 0; i < a_all_vert_ids.length(); i++)
	{
		MPoint pos_point1;
		meshFn.getPoint(a_all_vert_ids[i], pos_point1, MSpace::kWorld);
		a_allPoints[i] = pos_point1;
	}


	MPointArray b_allPoints;
	MIntArray b_all_vert_ids;
	meshFn.getPolygonVertices(b_id, b_all_vert_ids);
	b_allPoints.setLength(b_all_vert_ids.length());

	for (unsigned int j = 0; j < b_all_vert_ids.length(); j++)
	{
		MPoint pos_point2;
		meshFn.getPoint(b_all_vert_ids[j], pos_point2, MSpace::kWorld);
		b_allPoints[j] = pos_point2;
	}

	
	if (a_allPoints.length() != b_allPoints.length())
	{
		return false;
	}

	for (unsigned int i = 0; i < a_allPoints.length(); i++)
	{
		for (unsigned int j = 0; j < b_allPoints.length(); j++)
		{
			if (a_allPoints[i].isEquivalent(b_allPoints[j], threshold))
			{
				b_allPoints.remove(j);
				break;
			}
		}
	}

	if (b_allPoints.length()==0)
	{
		return true;
	}
	else
	{
		return false;
	}

}























