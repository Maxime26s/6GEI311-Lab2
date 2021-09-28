#include <dshow.h>
#include "playback.h"

#include <Python.h>

DShowPlayer* g_pPlayer = NULL;

void ManageInput(char ch) {
	switch (ch)
	{
	case L'p':
	case L'P':
		if (g_pPlayer->State() == STATE_RUNNING)
		{
			g_pPlayer->Pause();
		}
		else
		{
			g_pPlayer->Play();
		}
		break;
	case L'a':
	case L'A':
		g_pPlayer->FastForward();
		break;
	case L'r':
	case L'R':
		g_pPlayer->Back();
		break;
	case L'q':
	case L'Q':
		delete g_pPlayer;
		exit(0);
		break;
	}
}

static PyObject* initPlayer(PyObject* self, PyObject* args)
{
	g_pPlayer = new (std::nothrow) DShowPlayer(PyUnicode_AsUTF8(args));
	g_pPlayer->Pause();
	PyObject* python_val = Py_BuildValue("");

	return python_val;
}

static PyObject* sendInput(PyObject* self, PyObject* args)
{
	ManageInput(*PyUnicode_AsUTF8(args));
	PyObject* python_val = Py_BuildValue("");

	return python_val;
}

static PyMethodDef VideoPlayerMethods[] = {
	{"initPlayer", (PyCFunction)initPlayer, METH_O, "Shows the video player."},
	{"sendInput", (PyCFunction)sendInput, METH_O, "Sends input to the video player."},
	{nullptr, nullptr, 0, nullptr}        /* Sentinel */
};

static struct PyModuleDef videoplayermodule = {
	PyModuleDef_HEAD_INIT,
	"Lab2",   /* name of module */
	nullptr, /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
				 or -1 if the module keeps state in global variables. */
	VideoPlayerMethods
};

PyMODINIT_FUNC PyInit_Lab2(void)
{
	return PyModule_Create(&videoplayermodule);
}
