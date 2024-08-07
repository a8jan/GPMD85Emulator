/*	UserInterfaceData.h
	Copyright (c) 2011-2024 Martin Borik <mborik@users.sourceforge.net>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
//-----------------------------------------------------------------------------
#include "UserInterfaceData.h"
#include "Emulator.h"
//-----------------------------------------------------------------------------
const char *dcb_tape_save_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = TapeBrowser->tapeChanged;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_tape_noempty_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = (TapeBrowser->totalBlocks > 0);
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_tape_contblk_state(GUI_MENU_ENTRY *ptr)
{
	if (TapeBrowser->totalBlocks > 0) {
		int f = TapeBrowser->Selection->first,
			l = TapeBrowser->Selection->last,
			i = (ptr->action & 0x1ff);

		ptr->enabled = TapeBrowser->Selection->continuity;
		if (TapeBrowser->Selection->total == 0) {
			ptr->enabled = true;
			f = l = GUI->tapeDialog->popup.hilite;
		}

		if ((i == SDL_SCANCODE_UP && f == 0) ||
			(i == SDL_SCANCODE_DOWN && l == (TapeBrowser->totalBlocks - 1)))
			ptr->enabled = false;
	}

	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_view_size_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = (Settings->Screen->size == (TDisplayMode) ptr->action);
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_view_cmod_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = (Settings->Screen->colorProfile == (TColorProfile) ptr->action);
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_view_cpal_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = (Settings->Screen->colorPalette == (TColorPalette) ptr->action);
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_view_ccol_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = (Settings->Screen->colorPalette == CL_DEFINED);
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_view_sclr_state(GUI_MENU_ENTRY *ptr)
{
	if (ptr->action == (WORD) -1)
		ptr->state = Settings->Screen->lcdMode;
	else if (!Settings->Screen->lcdMode)
		ptr->state = (Settings->Screen->halfPass == (THalfPassMode) ptr->action);
	else
		ptr->state = false;

	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_view_brdr_state(GUI_MENU_ENTRY *ptr)
{
	if (Settings->Screen->size == DM_FULLSCREEN) {
		ptr->enabled = false;
		return NULL;
	}
	else {
		ptr->enabled = true;
		ptr->action = Settings->Screen->border;
		sprintf((char *) uicch, "%dx", ptr->action);
		return uicch;
	}
}
//-----------------------------------------------------------------------------
const char *dcb_snd_mute_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->Sound->mute;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_snd_volume_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = !Settings->Sound->mute;
	ptr->action = Settings->Sound->volume;
	sprintf((char *) uicch, "%d", ptr->action);
	return uicch;
}
//-----------------------------------------------------------------------------
const char *dcb_kbd_xchg_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->Keyboard->changeZY;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_kbd_nums_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->Keyboard->useNumpad;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_kbd_mato_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->Keyboard->useMatoCtrl;
	ptr->enabled = (Settings->CurrentModel->type == CM_MATO);
	return "for Ma\213o";
}
//-----------------------------------------------------------------------------
const char *dcb_emu_pause_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->isPaused;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_emu_speed_state(GUI_MENU_ENTRY *ptr)
{
	ptr->action = (WORD) (Settings->emulationSpeed * 100.0f);
	sprintf((char *) uicch, "%d%%", ptr->action);
	return uicch;
}
//-----------------------------------------------------------------------------
const char *dcb_emu_focus_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->pauseOnFocusLost;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_emu_asave_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = !Settings->fixedSettings;
	if (ptr->type == MI_CHECKBOX)
		ptr->state = Settings->autosaveSettings;

	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_machine_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = (Settings->CurrentModel->type == (TComputerModel) ptr->action);
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_mem_file_state(GUI_MENU_ENTRY *ptr)
{
	return ExtractFileName(Settings->CurrentModel->romFile);
}
//-----------------------------------------------------------------------------
const char *dcb_mem_rmod_state(GUI_MENU_ENTRY *ptr)
{
	switch (Settings->CurrentModel->type) {
		case CM_V1:
		case CM_V2:
		case CM_V2A:
		case CM_V3:
			ptr->enabled = true;
			ptr->state = Settings->CurrentModel->romModuleInserted;
			break;

		default:
			ptr->enabled = ptr->state = false;
			break;
	}

	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_mem_mrm_file_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = Settings->CurrentModel->romModuleInserted && Settings->CurrentModel->megaModuleEnabled;
	return ExtractFileName(Settings->CurrentModel->mrmFile);
}
//-----------------------------------------------------------------------------
const char *dcb_mem_mrm_state(GUI_MENU_ENTRY *ptr)
{
	switch (Settings->CurrentModel->type) {
		case CM_V1:
		case CM_V2:
		case CM_V2A:
		case CM_V3:
			ptr->enabled = Settings->CurrentModel->romModuleInserted;
			ptr->state = Settings->CurrentModel->megaModuleEnabled;
			break;

		default:
			ptr->enabled = ptr->state = false;
			break;
	}

	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_mem_mrmpage_state(GUI_MENU_ENTRY *ptr)
{
	int page = Emulator->ActionMegaModulePage();
	if (page < 0) {
		ptr->enabled = false;
		ptr->action = 0;
		*((char *) uicch) = '\0';
	}
	else {
		ptr->enabled = true;
		ptr->action = (WORD) page;
		sprintf((char *) uicch, "%d", page);
	}

	return uicch;
}
//-----------------------------------------------------------------------------
const char *dcb_mem_rpkg_state(GUI_MENU_ENTRY *ptr)
{
	if (gui_rom_packages == NULL) {
		int c = Settings->romPackagesCount, i;
		if (c > 20)
			c = 20;

		gui_rom_packages = new GUI_MENU_ENTRY[c + 2];
		gui_rom_packages[0].type = MI_TITLE;
		gui_rom_packages[0].text = "ROM PACKAGES";

		for (i = 0; i < c; i++) {
			gui_rom_packages[i + 1].type = MI_RADIO;
			gui_rom_packages[i + 1].text = Settings->RomPackages[i]->name;
			gui_rom_packages[i + 1].hotkey = NULL;
			gui_rom_packages[i + 1].submenu = NULL;
			gui_rom_packages[i + 1].key = -1;
			gui_rom_packages[i + 1].enabled = true;
			gui_rom_packages[i + 1].state = false;
			gui_rom_packages[i + 1].action = i;
			gui_rom_packages[i + 1].detail = dcb_rom_pckg_state;
			gui_rom_packages[i + 1].callback = ccb_rom_pckg;
		}

		gui_rom_packages[i + 1].type = MENU_END;
		ptr->submenu = gui_rom_packages;
	}

	switch (Settings->CurrentModel->type) {
		case CM_V1:
		case CM_V2:
		case CM_V2A:
		case CM_V3:
			ptr->enabled = Settings->CurrentModel->romModuleInserted &&
			              !Settings->CurrentModel->megaModuleEnabled;
			return ExtractFileName(Settings->CurrentModel->romModule->name);

		default:
			ptr->enabled = false;
			return NULL;
	}
}
//-----------------------------------------------------------------------------
const char *dcb_rom_pckg_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = (Settings->CurrentModel->romModule->name == ptr->text);
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_mem_x256k_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->CurrentModel->ramExpansion256k;
	ptr->enabled = (Settings->CurrentModel->type == CM_V2A ||
	                Settings->CurrentModel->type == CM_V3);
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_mem_m3cmp_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->CurrentModel->compatibilityMode;
	ptr->enabled = (Settings->CurrentModel->type == CM_V3);
	return "for PMD 85-3";
}
//-----------------------------------------------------------------------------
const char *dcb_mem_spl8k_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->CurrentModel->romSplit8kMode;
	ptr->enabled = Settings->CurrentModel->type <= CM_V2A;
	return "on 8000h/A000h";
}
//-----------------------------------------------------------------------------
const char *dcb_p32_conn_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->PMD32->connected;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_p32_imgs_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = Settings->PMD32->connected;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_p32_extc_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->PMD32->extraCommands;
	ptr->enabled = Settings->PMD32->connected;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_p32_sdcd_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = Settings->PMD32->connected && Settings->PMD32->extraCommands;
	return ExtractFileName(Settings->PMD32->sdRoot);
}
//-----------------------------------------------------------------------------
const char *dcb_p32_imgd_state(GUI_MENU_ENTRY *ptr)
{
	const char *ret = NULL;

	switch (ptr->action & 0x3FFF) {
		case 1:
			ptr->state = Settings->PMD32->driveA.writeProtect;
			ret = ExtractFileName(Settings->PMD32->driveA.image);
			break;
		case 2:
			ptr->state = Settings->PMD32->driveB.writeProtect;
			ret = ExtractFileName(Settings->PMD32->driveB.image);
			break;
		case 3:
			ptr->state = Settings->PMD32->driveC.writeProtect;
			ret = ExtractFileName(Settings->PMD32->driveC.image);
			break;
		case 4:
			ptr->state = Settings->PMD32->driveD.writeProtect;
			ret = ExtractFileName(Settings->PMD32->driveD.image);
			break;
		default:
			break;
	}

	return ret;
}
//-----------------------------------------------------------------------------
const char *dcb_joy_conn_state(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	if (ptr->action == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;
	else if (Settings->PMD32->connected) {
		gpio->connected = false;
		if (ptr->submenu)
			ptr->enabled = false;
		else
			ptr->state = false;
		return NULL;
	}
	if (ptr->submenu)
		ptr->enabled = gpio->connected;
	else
		ptr->state = gpio->connected;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_joy_menu_state(GUI_MENU_ENTRY *ptr)
{
	dcb_joy_conn_state(ptr);
	if (!Emulator->ActionJoyControllers())
		ptr->enabled = false;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_joy_type_state(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	if ((ptr->action >> 8) == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;
	BYTE type = (ptr->action & 0xFF);
	ptr->state = (gpio->type == (TJoyType) type);

	static const char *comments[4] = {
		/* JT_KEYS */
		NULL,
		/* JT_POV */
		"DPAD+A",
		/* JT_AXES */
		"Stick+A",
		/* JT_BUTTONS */
		"ABXY+LB"
	};

	return comments[type];
}
//-----------------------------------------------------------------------------
const char *dcb_joy_select_state(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	if (ptr->action == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;

	SDL_GameController **controllers;
	int devCount = Emulator->ActionJoyControllers(&controllers, false);
	ptr->enabled = (devCount > 1);
	ptr->text = "";

	static char *controllerName = NULL;

	*((char *) uicch) = '\0';
	if (gpio->guid && gpio->guid[0] != '\0') {
		for (int jj = 0; jj < devCount; jj++) {
			if (strcasecmp(gpio->guid, SDL_GameControllerGetSerial(controllers[jj])) == 0) {
				const char *name = SDL_GameControllerName(controllers[jj]);
				if (name != NULL) {
					if (controllerName != NULL)
						delete controllerName;
					controllerName = new char[22];

					if (strlen(name) > 20) {
						strncpy(controllerName, name, 20);
						controllerName[20] = '\205';
						controllerName[21] = '\0';
					}
					else
						strcpy(controllerName, SDL_GameControllerName(controllers[jj]));

					ptr->text = controllerName;
				}

				sprintf((char *) uicch, "%d", jj + 1);
				break;
			}
		}
	}
	else {
		ptr->enabled = true;
		ptr->text = "SELECT\205";
		return "\205";
	}

	return uicch;
}
//-----------------------------------------------------------------------------
const char *dcb_joy_sens_state(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	if ((ptr->action >> 8) == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;
	ptr->enabled = gpio->type == JT_AXES;
	sprintf((char *) uicch, "%d", gpio->sensitivity);
	return uicch;
}
//-----------------------------------------------------------------------------
const char *dcb_mouse_conn_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->Mouse->type == MT_M602;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_mouse_cursor_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->Mouse->hideCursor;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_mif85_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->Sound->ifMIF85;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_blk_file_state(GUI_MENU_ENTRY *ptr)
{
	return ExtractFileName(Settings->MemoryBlock->fileName);
}
//-----------------------------------------------------------------------------
const char *dcb_blk_strt_state(GUI_MENU_ENTRY *ptr)
{
	ptr->action = (WORD) Settings->MemoryBlock->start;
	sprintf((char *) uicch, Settings->MemoryBlock->hex ? "#%04X" : "%d", ptr->action);
	return uicch;
}
//-----------------------------------------------------------------------------
const char *dcb_blk_leng_state(GUI_MENU_ENTRY *ptr)
{
	ptr->action = (WORD) Settings->MemoryBlock->length;
	sprintf((char *) uicch, Settings->MemoryBlock->hex ? "#%04X" : "%d", ptr->action);
	return uicch;
}
//-----------------------------------------------------------------------------
const char *dcb_blk_hexa_state(GUI_MENU_ENTRY *ptr)
{
	ptr->state = Settings->MemoryBlock->hex;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_blk_roma_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = (Settings->CurrentModel->type == CM_V2A
	             || Settings->CurrentModel->type == CM_V3
	             || Settings->CurrentModel->type == CM_C2717);
	ptr->state = Settings->MemoryBlock->rom;
	return NULL;
}
//-----------------------------------------------------------------------------
const char *dcb_blk_rmap_state(GUI_MENU_ENTRY *ptr)
{
	ptr->enabled = (Settings->CurrentModel->type == CM_C2717);
	ptr->state = Settings->MemoryBlock->remapping;
	return NULL;
}
//-----------------------------------------------------------------------------
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//-----------------------------------------------------------------------------
bool ccb_tape_command(GUI_MENU_ENTRY *ptr) { return false; }
//-----------------------------------------------------------------------------
bool ccb_tape_new(GUI_MENU_ENTRY *ptr)
{
	Emulator->ActionTapeNew();
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_fileselector(GUI_MENU_ENTRY *ptr)
{
	switch (ptr->action) {
		case 1:
			Emulator->ActionTapeLoad();
			break;

		case 2:
			Emulator->ActionTapeSave();
			break;

		case 3:
			Emulator->ActionSnapLoad();
			break;

		case 4:
			Emulator->ActionSnapSave();
			break;

		case 5:
			Emulator->ActionROMLoad();
			break;

		case 6:
			Emulator->ActionMegaRomLoad();
			break;

		case 7:
			Emulator->ActionRawFile(ptr->state);
			break;

		case 8:
			Emulator->ActionTapeLoad(true);
			break;

		default:
			break;
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_view_size(GUI_MENU_ENTRY *ptr)
{
	Settings->Screen->size = (TDisplayMode) ptr->action;
	GUI->uiSetChanges |= PS_SCREEN_SIZE;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_view_brdr(GUI_MENU_ENTRY *ptr)
{
	WORD value = ((ptr) ? ptr->action : Settings->Screen->border);

	sprintf(msgbuffer, "%d", value);
	if (GUI->EditBox("CHANGE BORDER SIZE:", "(multiples of 8px)", msgbuffer, 1, true) == 1) {
		value = strtol(msgbuffer, NULL, 10);
		if (value == 0 && msgbuffer[0] != '0')
			value = -1;

		if (value >= 0 && value <= 9) {
			Settings->Screen->border = (BYTE) value;
			GUI->uiSetChanges |= PS_SCREEN_SIZE;
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_view_cmod(GUI_MENU_ENTRY *ptr)
{
	Settings->Screen->colorProfile = (TColorProfile) ptr->action;
	GUI->uiSetChanges |= PS_SCREEN_MODE;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_view_cpal(GUI_MENU_ENTRY *ptr)
{
	Settings->Screen->colorPalette = (TColorPalette) ptr->action;
	GUI->uiSetChanges |= PS_SCREEN_MODE;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_view_sclr(GUI_MENU_ENTRY *ptr)
{
	if (ptr->action == (WORD) -1) {
		Settings->Screen->lcdMode = true;
		Settings->Screen->halfPass = HP_OFF;
	}
	else {
		Settings->Screen->lcdMode = false;
		Settings->Screen->halfPass = (THalfPassMode) ptr->action;
	}

	GUI->uiSetChanges |= PS_SCREEN_MODE;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_snd_mute(GUI_MENU_ENTRY *ptr)
{
	Settings->Sound->mute = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_SOUND;

	ptr++;
	ptr->enabled = !Settings->Sound->mute;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_snd_volume(GUI_MENU_ENTRY *ptr)
{
	WORD value = ((ptr) ? ptr->action : Settings->Sound->volume);

	sprintf(msgbuffer, "%d", value);
	if (GUI->EditBox("CHANGE VOLUME:", "(min=2, max=127)", msgbuffer, 3, true) == 1) {
		value = strtol(msgbuffer, NULL, 10);
		if (value > 1 && value <= 127) {
			Settings->Sound->volume = (BYTE) value;
			GUI->uiSetChanges |= PS_SOUND;
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_kbd_xchg(GUI_MENU_ENTRY *ptr)
{
	Settings->Keyboard->changeZY = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_CONTROLS;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_kbd_nums(GUI_MENU_ENTRY *ptr)
{
	Settings->Keyboard->useNumpad = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_CONTROLS;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_kbd_mato(GUI_MENU_ENTRY *ptr)
{
	Settings->Keyboard->useMatoCtrl = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_CONTROLS;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_emu_pause(GUI_MENU_ENTRY *ptr)
{
	Settings->isPaused = (ptr->state = !ptr->state);
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_emu_speed(GUI_MENU_ENTRY *ptr)
{
	WORD value = ((ptr) ? ptr->action : (Settings->emulationSpeed * 100.0f));

	sprintf(msgbuffer, "%d", value);
	if (GUI->EditBox("EMULATION SPEED:", "(enter 10% to 1000%)", msgbuffer, 4, true) == 1) {
		value = strtol(msgbuffer, NULL, 10);
		if (value == 0 && msgbuffer[0] != '0')
			value = 100;
		else if (value < 10 && value > 1000)
			value = 100;

		Settings->emulationSpeed = ((double) value / 100.0f);
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_emu_reset(GUI_MENU_ENTRY *ptr)
{
	GUI->uiCallback.connect(Emulator, &TEmulator::ActionReset);
	GUI->uiSetChanges |= PS_CLOSEALL;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_emu_hardreset(GUI_MENU_ENTRY *ptr)
{
	GUI->uiCallback.connect(Emulator, &TEmulator::ActionHardReset);
	GUI->uiSetChanges |= PS_CLOSEALL;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_emu_focus(GUI_MENU_ENTRY *ptr)
{
	Settings->pauseOnFocusLost = (ptr->state = !ptr->state);
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_emu_asave(GUI_MENU_ENTRY *ptr)
{
	Settings->autosaveSettings = (ptr->state = !ptr->state);
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_emu_saves(GUI_MENU_ENTRY *ptr)
{
	Settings->storeSettings();
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_machine(GUI_MENU_ENTRY *ptr)
{
	for (int i = 0; i < Settings->modelsCount; i++) {
		if (Settings->AllModels[i]->type == (TComputerModel) ptr->action) {
			Settings->CurrentModel = Settings->AllModels[i];
			break;
		}
	}

	GUI->uiSetChanges |= PS_MACHINE | PS_CLOSEALL;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_mem_rmod(GUI_MENU_ENTRY *ptr)
{
	Settings->CurrentModel->romModuleInserted = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_MACHINE | PS_PERIPHERALS;

	while ((++ptr)->type != MENU_END)
		if (ptr->detail)
			ptr->detail(ptr);

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_mem_mrm(GUI_MENU_ENTRY *ptr)
{
	Settings->CurrentModel->megaModuleEnabled = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_MACHINE | PS_PERIPHERALS;

	--ptr;
	while (ptr->type != MENU_END) {
		if (ptr->detail)
			ptr->detail(ptr);
		ptr++;
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_mem_mrmpage(GUI_MENU_ENTRY *ptr)
{
	WORD value = ptr ? ptr->action : (WORD) Emulator->ActionMegaModulePage();

	sprintf(msgbuffer, "%d", value);
	if (GUI->EditBox("MEGAMODULE PAGE:", "(enter 0 to 256)", msgbuffer, 3, true) == 1) {
		value = strtol(msgbuffer, NULL, 10);
		if (value <= MEGA_MODULE_MAX_PAGES) {
			Emulator->ActionMegaModulePage(true, (BYTE) value);
			GUI->uiSetChanges |= PS_CLOSEALL;
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_rom_pckg(GUI_MENU_ENTRY *ptr)
{
	Settings->CurrentModel->romModule = Settings->RomPackages[ptr->action];
	GUI->uiSetChanges |= PS_MACHINE | PS_PERIPHERALS;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_mem_x256k(GUI_MENU_ENTRY *ptr)
{
	Settings->CurrentModel->ramExpansion256k = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_MACHINE;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_mem_m3cmp(GUI_MENU_ENTRY *ptr)
{
	Settings->CurrentModel->compatibilityMode = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_MACHINE;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_mem_spl8k(GUI_MENU_ENTRY *ptr)
{
	Settings->CurrentModel->romSplit8kMode = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_MACHINE;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_p32_imgd(GUI_MENU_ENTRY *ptr)
{
	if (ptr->action >= 0xC000) {
		ptr->action ^= 0x4000;
		ptr->state = !ptr->state;

		switch (ptr->action & 0xF) {
			case 1:
				Settings->PMD32->driveA.writeProtect = ptr->state;
				break;
			case 2:
				Settings->PMD32->driveB.writeProtect = ptr->state;
				break;
			case 3:
				Settings->PMD32->driveC.writeProtect = ptr->state;
				break;
			case 4:
				Settings->PMD32->driveD.writeProtect = ptr->state;
				break;
			default:
				break;
		}

		GUI->uiSetChanges |= PS_PERIPHERALS;
		return false;
	}
	else
		Emulator->ActionPMD32LoadDisk(ptr->action & 0xF);

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_p32_conn(GUI_MENU_ENTRY *ptr)
{
	Settings->PMD32->connected = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_PERIPHERALS;

	while ((++ptr)->type != MENU_END)
		if (ptr->detail)
			ptr->detail(ptr);

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_p32_extc(GUI_MENU_ENTRY *ptr)
{
	Settings->PMD32->extraCommands = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_PERIPHERALS;

	ptr++;
	ptr->enabled = Settings->PMD32->extraCommands;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_joy_conn(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	if (ptr->action == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;
	gpio->connected = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_PERIPHERALS;

	while ((++ptr)->type != MENU_END)
		if (ptr->detail)
			ptr->detail(ptr);

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_joy_type(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	if ((ptr->action >> 8) == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;

	TJoyType newType = (TJoyType) (ptr->action & 0xFF);
	if (!gpio->connected || gpio->type == newType)
		return true;

	gpio->type = newType;

	while ((--ptr)->type != MI_TITLE);
	while ((++ptr)->type != MENU_END)
		if (ptr->detail)
			ptr->detail(ptr);

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_joy_keyset(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	if ((ptr->action >> 8) == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;

	static const WORD joyKeymaps[4][5] = {
		/* JKM_NUMPAD */
		{ SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_6, SDL_SCANCODE_KP_0 },
		/* JKM_CURSORS */
		{ SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_LCTRL },
		/* JKM_QAOP */
		{ SDL_SCANCODE_Q, SDL_SCANCODE_A, SDL_SCANCODE_O, SDL_SCANCODE_P, SDL_SCANCODE_SPACE },
		/* JKM_WASD */
		{ SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_C }
	};

	int keymap = (ptr->action & 0xFF);
	gpio->ctrlUp = joyKeymaps[keymap][0];
	gpio->ctrlDown = joyKeymaps[keymap][1];
	gpio->ctrlLeft = joyKeymaps[keymap][2];
	gpio->ctrlRight = joyKeymaps[keymap][3];
	gpio->ctrlFire = joyKeymaps[keymap][4];

	return true;
}
//-----------------------------------------------------------------------------
bool ccb_joy_sens(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	WORD gpio_w = (ptr->action & 0x100);
	if ((ptr->action >> 8) == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;

	int value = gpio->sensitivity;

	sprintf(msgbuffer, "%d", value);
	if (GUI->EditBox("AXIS SENSITIVITY:", "(lower is more sensitive)", msgbuffer, 2, true) == 1) {
		value = strtol(msgbuffer, NULL, 10);
		if (value >= 1 && value < 100) {
			gpio->sensitivity = value;
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_joy_select(GUI_MENU_ENTRY *ptr)
{
	TSettings::SetJoystickGPIO *gpio = Settings->Joystick->GPIO0;
	if (ptr->action == GP_GPIO_1)
		gpio = Settings->Joystick->GPIO1;

	SDL_GameController **controllers;
	int devCount = Emulator->ActionJoyControllers(&controllers, false);

	int value = 0;
	if (gpio->guid && gpio->guid[0] != '\0') {
		for (int jj = 0; jj < devCount; jj++) {
			if (strcasecmp(gpio->guid, SDL_GameControllerGetSerial(controllers[jj])) == 0) {
				value = jj + 1;
				break;
			}
		}
	}

	if (value == 0)
		value = devCount;

	char *input = (char *) uicch;
	sprintf(msgbuffer, "(total controllers = %d)", devCount);
	sprintf(input, "%d", value);
	if (GUI->EditBox("CONTROLLER NUMBER:", msgbuffer, input, 1, true) == 1) {
		value = strtol(input, NULL, 10);
		if (value >= 1 && value <= devCount) {
			const char *guid = SDL_GameControllerGetSerial(controllers[value - 1]);
			delete gpio->guid;
			gpio->guid = new char[strlen(guid) + 1];
			strcpy(gpio->guid, guid);

			Emulator->ActionJoyControllers(NULL, true);
			ptr->detail(ptr);
		}
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_mouse_conn(GUI_MENU_ENTRY *ptr)
{
	Settings->Mouse->type = (ptr->state = !ptr->state) ? MT_M602 : MT_NONE;
	GUI->uiSetChanges |= PS_PERIPHERALS;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_mouse_cursor(GUI_MENU_ENTRY *ptr)
{
	Emulator->ActionHideCursor(ptr->state = !ptr->state);
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_mif85(GUI_MENU_ENTRY *ptr)
{
	Settings->Sound->ifMIF85 = (ptr->state = !ptr->state);
	GUI->uiSetChanges |= PS_PERIPHERALS;
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_blk_strt(GUI_MENU_ENTRY *ptr)
{
	int value = ((ptr) ? ptr->action : Settings->MemoryBlock->start);

	sprintf(msgbuffer, Settings->MemoryBlock->hex ? "#%04X" : "%d", value);
	if (GUI->EditBox("CHANGE START ADDRESS:", NULL, msgbuffer, 5, false) == 1) {
		if (msgbuffer[0] == '#') {
			value = strtol(msgbuffer + 1, NULL, 16);
			if (value == 0 && msgbuffer[1] != '0')
				value = -1;
		}
		else {
			value = strtol(msgbuffer, NULL, 10);
			if (value == 0 && msgbuffer[0] != '0')
				value = -1;
		}

		if (value >= 0 && value < 65536)
			Settings->MemoryBlock->start = (WORD) value;
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_blk_leng(GUI_MENU_ENTRY *ptr)
{
	int value = ((ptr) ? ptr->action : Settings->MemoryBlock->length);

	sprintf(msgbuffer, Settings->MemoryBlock->hex ? "#%04X" : "%d", value);
	if (GUI->EditBox("CHANGE LENGTH:", NULL, msgbuffer, 5, false) == 1) {
		if (msgbuffer[0] == '#') {
			value = strtol(msgbuffer + 1, NULL, 16);
			if (value == 0 && msgbuffer[1] != '0')
				value = -1;
		}
		else {
			value = strtol(msgbuffer, NULL, 10);
			if (value == 0 && msgbuffer[0] != '0')
				value = -1;
		}

		if (value >= 0 && value < 65536)
			Settings->MemoryBlock->length = (WORD) value;
	}

	return false;
}
//-----------------------------------------------------------------------------
bool ccb_blk_hexa(GUI_MENU_ENTRY *ptr)
{
	Settings->MemoryBlock->hex = (ptr->state = !ptr->state);
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_blk_roma(GUI_MENU_ENTRY *ptr)
{
	Settings->MemoryBlock->rom = (ptr->state = !ptr->state);
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_blk_rmap(GUI_MENU_ENTRY *ptr)
{
	Settings->MemoryBlock->remapping = (ptr->state = !ptr->state);
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_blk_exec(GUI_MENU_ENTRY *ptr)
{
	bool result = Emulator->ProcessRawFile(ptr->state);
	if (result)
		GUI->uiSetChanges |= PS_CLOSEALL;
	else
		GUI->MessageBox("FATAL ERROR!\nINVALID LENGTH OR CAN'T OPEN FILE!");

	return result;
}
//-----------------------------------------------------------------------------
bool ccb_tapebrowser(GUI_MENU_ENTRY *ptr)
{
	GUI->uiCallback.connect(Emulator, &TEmulator::ActionTapeBrowser);
	GUI->uiSetChanges |= PS_CLOSEALL;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_debugger(GUI_MENU_ENTRY *ptr)
{
	GUI->uiCallback.connect(Emulator, &TEmulator::ActionDebugger);
	GUI->uiSetChanges |= PS_CLOSEALL;
	return true;
}
//-----------------------------------------------------------------------------
bool ccb_about(GUI_MENU_ENTRY *ptr)
{
	GUI->AboutDialog();
	return false;
}
//-----------------------------------------------------------------------------
bool ccb_exit(GUI_MENU_ENTRY *ptr)
{
	Emulator->ActionExit();
	return true;
}
//-----------------------------------------------------------------------------
