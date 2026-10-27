#include "stdafx.h"
#include "EngineThreading.h"
#include "CustomHUD.h"
#include "IGame_Persistent.h"
#include "IGame_Level.h"
#include "Rain.h"
#include "../Include/xrRender/RenderVisual.h"
#include "../Include/xrRender/ParticleCustom.h"
#include "../xrCore/discord/discord.h"
#include "../xrCore/Save/SaveManager.h"

void XRay::Engine::PreRenderThread()
{
	{
		PROF_EVENT("Discord Sync");
		g_Discord.Update();
	}

	{
		PROF_EVENT("seqParallelRender");
		for (auto& it : Device.seqParallelRender)
			it();
	}

	if (g_pGamePersistent && !g_pGamePersistent->m_pMainMenu->IsActive())
	{
		if (g_pGamePersistent->pEnvironment && g_pGamePersistent->pEnvironment->eff_Rain)
			g_pGamePersistent->pEnvironment->eff_Rain->UpdateItems();

		g_pGamePersistent->UpdateParticles();
	}
}

void XRay::Engine::GameThread()
{
	// Device.GCThread.Wait();

	{
		PROF_EVENT("g_hud OnFrameMT");
		if (g_hud)
		{
			g_hud->OnFrameMT();
		}
	}

	{
		PROF_EVENT("SoundEvent_Dispatch");
		if (g_pGameLevel && g_pGameLevel->bReady)
		{
			g_pGameLevel->SoundEvent_Dispatch();
		}
	}

	{
		PROF_EVENT("Save Writing");
		while (auto task = CSaveManager::GetInstance().PopSaveTask())
		{
			Device.async_tasks.run([=]()
			{
				PROF_START_THREAD("Async Task");
				PROF_EVENT("Save writing task");
				task->WriteSavedDataImpl();
				xr_delete(task);
				PROF_STOP_THREAD();
			});
		}
	}

	{
		PROF_EVENT("Sheduler");
		if (!Device.Paused())
		{
			::Engine.Sheduler.Update();
		}
	}

	{
		PROF_EVENT("seqParallel");
		u32 Processed = 0;
		for (;;)
		{
			xr_delegate<void()> Task;
			{
				xrCriticalSectionGuard Guard(Device.seqParallelLock);
				if (Processed >= Device.seqParallel.size())
				{
					break;
				}
				Task = Device.seqParallel[Processed];
			}
			++Processed;
			Task();
		}

		xrCriticalSectionGuard Guard(Device.seqParallelLock);
		const u32 Count = std::min<u32>(Processed, u32(Device.seqParallel.size()));
		Device.seqParallel.erase(Device.seqParallel.begin(), Device.seqParallel.begin() + Count);
	}

	{
		PROF_EVENT("seqFrameMT");
		Device.seqFrameMT.Process<&pureFrame::OnFrame>();
	}

	// Device.GCThread.Run();

	if (g_pGamePersistent != nullptr)
	{
		g_pGamePersistent->OnFrameMT();
	}

	if (Device.LuaGC)
	{
		PROF_EVENT("LuaGC");
		Device.LuaGC();
	}

	if (::Sound != nullptr)
	{
		Device.Statistic->Sound.Begin();
		::Sound->update(Device.vCameraPosition_saved, Device.vCameraDirection_saved, Device.vCameraTop_saved);
		Device.Statistic->Sound.End();
	}

	Device.SecondaryTasks.wait();
}

extern volatile bool quiting;

void XRay::Engine::GCThread()
{
	if (Device.LuaGC != nullptr)
	{
		PROF_EVENT("LuaGC");
		Device.LuaGC();
	}
}
