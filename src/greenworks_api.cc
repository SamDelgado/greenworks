// Copyright (c) 2014 Greenheart Games Pty. Ltd. All rights reserved.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include <string>
#include <sstream>

#include "nan.h"
#include "steam/steam_api.h"
#include "v8.h"

#include "greenworks_async_workers.h"
#include "greenworks_utils.h"

namespace {

v8::Local<v8::Object> GetSteamUserCountType(int type_id) {
  v8::Local<v8::Object> account_type = NanNew<v8::Object>();
  std::string name;
  switch (type_id){
    case k_EAccountTypeAnonGameServer:
      name = "k_EAccountTypeAnonGameServer";
      break;
    case k_EAccountTypeAnonUser:
      name = "k_EAccountTypeAnonUser";
      break;
    case k_EAccountTypeChat:
      name = "k_EAccountTypeChat";
      break;
    case k_EAccountTypeClan:
      name = "k_EAccountTypeClan";
      break;
    case k_EAccountTypeConsoleUser:
      name = "k_EAccountTypeConsoleUser";
      break;
    case k_EAccountTypeContentServer:
      name = "k_EAccountTypeContentServer";
      break;
    case k_EAccountTypeGameServer:
      name = "k_EAccountTypeGameServer";
      break;
    case k_EAccountTypeIndividual:
      name = "k_EAccountTypeIndividual";
      break;
    case k_EAccountTypeInvalid:
      name = "k_EAccountTypeInvalid";
      break;
    case k_EAccountTypeMax:
      name = "k_EAccountTypeMax";
      break;
    case k_EAccountTypeMultiseat:
      name = "k_EAccountTypeMultiseat";
      break;
    case k_EAccountTypePending:
      name = "k_EAccountTypePending";
      break;
  }
  account_type->Set(NanNew("name"), NanNew(name));
  account_type->Set(NanNew("value"), NanNew(type_id));
  return account_type;
}

NAN_METHOD(InitAPI) {
  NanScope();

  bool success = SteamAPI_Init();

  if (success) {
    ISteamUserStats* stream_user_stats = SteamUserStats();
    stream_user_stats->RequestCurrentStats();
  }

  NanReturnValue(NanNew(success));
}

NAN_METHOD(GetSteamId) {
  NanScope();
  CSteamID user_id = SteamUser()->GetSteamID();
  v8::Local<v8::Object> flags = NanNew<v8::Object>();
  flags->Set(NanNew("anonymous"), NanNew(user_id.BAnonAccount()));
  flags->Set(NanNew("anonymousGameServer"),
      NanNew(user_id.BAnonGameServerAccount()));
  flags->Set(NanNew("anonymousGameServerLogin"),
      NanNew(user_id.BBlankAnonAccount()));
  flags->Set(NanNew("anonymousUser"), NanNew(user_id.BAnonUserAccount()));
  flags->Set(NanNew("chat"), NanNew(user_id.BChatAccount()));
  flags->Set(NanNew("clan"), NanNew(user_id.BClanAccount()));
  flags->Set(NanNew("consoleUser"), NanNew(user_id.BConsoleUserAccount()));
  flags->Set(NanNew("contentServer"), NanNew(user_id.BContentServerAccount()));
  flags->Set(NanNew("gameServer"), NanNew(user_id.BGameServerAccount()));
  flags->Set(NanNew("individual"), NanNew(user_id.BIndividualAccount()));
  flags->Set(NanNew("gameServerPersistent"),
      NanNew(user_id.BPersistentGameServerAccount()));
  flags->Set(NanNew("lobby"), NanNew(user_id.IsLobby()));

  v8::Local<v8::Object> result = NanNew<v8::Object>();
  result->Set(NanNew("flags"), flags);
  result->Set(NanNew("type"), GetSteamUserCountType(user_id.GetEAccountType()));
  result->Set(NanNew("accountId"), NanNew<v8::Integer>(user_id.GetAccountID()));
  result->Set(NanNew("staticAccountId"),
              NanNew<v8::Integer>(user_id.GetStaticAccountKey()));
  result->Set(NanNew("isValid"), NanNew<v8::Integer>(user_id.IsValid()));
  result->Set(NanNew("level"), NanNew<v8::Integer>(
        SteamUser()->GetPlayerSteamLevel()));

  if (!SteamFriends()->RequestUserInformation(user_id, true)) {
    result->Set(NanNew("screenName"),
                NanNew(SteamFriends()->GetFriendPersonaName(user_id)));
  } else {
    std::ostringstream sout;
    sout << user_id.GetAccountID();
    result->Set(NanNew("screenName"), NanNew(sout.str()));
  }
  NanReturnValue(result);
}

NAN_METHOD(SaveTextToFile) {
  NanScope();

  if (args.Length() < 4) {
    NanThrowTypeError("Wrong numer of arguments, should be 4.");
    NanReturnUndefined();
  }

  std::string file_name(*(v8::String::Utf8Value(args[0])));
  std::string content(*(v8::String::Utf8Value(args[1])));
  NanCallback* successCallback = new NanCallback(args[2].As<v8::Function>());
  NanCallback* errorCallback = new NanCallback(args[3].As<v8::Function>());

  NanAsyncQueueWorker(new greenworks::FileSaveWorker(successCallback,
                                                     errorCallback,
                                                     file_name,
                                                     content));
  NanReturnUndefined();
}

NAN_METHOD(ReadTextFromFile) {
  NanScope();

  if (args.Length() < 3) {
    NanThrowTypeError("Wrong numer of arguments, should be 3.");
    NanReturnUndefined();
  }

  std::string file_name(*(v8::String::Utf8Value(args[0])));
  NanCallback* successCallback = new NanCallback(args[1].As<v8::Function>());
  NanCallback* errorCallback = new NanCallback(args[2].As<v8::Function>());

  NanAsyncQueueWorker(new greenworks::FileReadWorker(successCallback,
                                                     errorCallback,
                                                     file_name));
  NanReturnUndefined();
}

NAN_METHOD(IsCloudEnabled) {
  NanScope();
  ISteamRemoteStorage* steam_remote_storage = SteamRemoteStorage();
  NanReturnValue(NanNew<v8::Boolean>(
      steam_remote_storage->IsCloudEnabledForApp()));
}

NAN_METHOD(IsCloudEnabledForUser) {
  NanScope();
  ISteamRemoteStorage* steam_remote_storage = SteamRemoteStorage();
  NanReturnValue(NanNew<v8::Boolean>(
      steam_remote_storage->IsCloudEnabledForAccount()));
}

NAN_METHOD(EnableCloud) {
  NanScope();

  if (args.Length() < 1) {
    NanThrowTypeError("Wrong numer of arguments, should be 1.");
    NanReturnUndefined();
  }
  bool enable_flag = args[0]->BooleanValue();
  SteamRemoteStorage()->SetCloudEnabledForApp(enable_flag);
  NanReturnUndefined();
}

NAN_METHOD(GetCloudQuota) {
  NanScope();

  if (args.Length() < 2) {
    NanThrowTypeError("Wrong numer of arguments, should be 2.");
    NanReturnUndefined();
  }
  NanCallback* success_callback = new NanCallback(args[0].As<v8::Function>());
  NanCallback* error_callback = new NanCallback(args[1].As<v8::Function>());
  NanAsyncQueueWorker(new greenworks::CloudQuotaGetWorker(success_callback,
                                                          error_callback));
  NanReturnUndefined();
}

NAN_METHOD(ActivateAchievement) {
  NanScope();

  if (args.Length() < 3) {
    NanThrowTypeError("Wrong numer of arguments, should be 3.");
    NanReturnUndefined();
  }
  std::string achievement = (*(v8::String::Utf8Value(args[0])));
  NanCallback* success_callback = new NanCallback(args[1].As<v8::Function>());
  NanCallback* error_callback = new NanCallback(args[2].As<v8::Function>());
  NanAsyncQueueWorker(new greenworks::ActivateAchievementWorker(
      success_callback, error_callback, achievement));
  NanReturnUndefined();
}

NAN_METHOD(GetCurrentGameLanguage) {
  NanScope();
  NanReturnValue(NanNew(SteamApps()->GetCurrentGameLanguage()));
}

NAN_METHOD(GetCurrentUILanguage) {
  NanScope();
  NanReturnValue(NanNew(SteamUtils()->GetSteamUILanguage()));
}

// TODO: Implement get game install directory.
NAN_METHOD(GetCurrentGameInstallDir) {
  NanScope();
  NanReturnValue(NanNew("NOT IMPLEMENTED"));
}

NAN_METHOD(GetNumberOfPlayers) {
  NanScope();
  if (args.Length() < 2) {
    NanThrowTypeError("Wrong numer of arguments, should be 2.");
    NanReturnUndefined();
  }
  NanCallback* success_callback = new NanCallback(args[0].As<v8::Function>());
  NanCallback* error_callback = new NanCallback(args[1].As<v8::Function>());
  NanAsyncQueueWorker(new greenworks::GetNumberOfPlayersWorker(
      success_callback, error_callback));
  NanReturnUndefined();
}

NAN_METHOD(FileShare) {
  NanScope();

  if (args.Length() < 3) {
    NanThrowTypeError("Wrong numer of arguments, should be 3.");
    NanReturnUndefined();
  }
  std::string file_name(*(v8::String::Utf8Value(args[0])));
  NanCallback* success_callback = new NanCallback(args[1].As<v8::Function>());
  NanCallback* error_callback = new NanCallback(args[2].As<v8::Function>());

  NanAsyncQueueWorker(new greenworks::FileShareWorker(
      success_callback, error_callback, file_name));
  NanReturnUndefined();
}

void init(v8::Handle<v8::Object> exports) {
  // Common APIs.
  exports->Set(NanNew("initAPI"),
               NanNew<v8::FunctionTemplate>(InitAPI)->GetFunction());
  exports->Set(NanNew("getSteamId"),
               NanNew<v8::FunctionTemplate>(GetSteamId)->GetFunction());
  // File related APIs.
  exports->Set(NanNew("saveTextToFile"),
               NanNew<v8::FunctionTemplate>(SaveTextToFile)->GetFunction());
  exports->Set(NanNew("readTextFromFile"),
               NanNew<v8::FunctionTemplate>(ReadTextFromFile)->GetFunction());
  // Cloud related APIs.
  exports->Set(NanNew("isCloudEnabled"),
               NanNew<v8::FunctionTemplate>(IsCloudEnabled)->GetFunction());
  exports->Set(NanNew("isCloudEnabledForUser"),
               NanNew<v8::FunctionTemplate>(
                   IsCloudEnabledForUser)->GetFunction());
  exports->Set(NanNew("enableCloud"),
               NanNew<v8::FunctionTemplate>(EnableCloud)->GetFunction());
  exports->Set(NanNew("getCloudQuota"),
               NanNew<v8::FunctionTemplate>(GetCloudQuota)->GetFunction());
  // Achievement related APIs.
  exports->Set(NanNew("activateAchievement"),
               NanNew<v8::FunctionTemplate>(
                   ActivateAchievement)->GetFunction());
  // Game setting related APIs.
  exports->Set(NanNew("getCurrentGameLanguage"),
                      NanNew<v8::FunctionTemplate>(
                          GetCurrentGameLanguage)->GetFunction());
  exports->Set(NanNew("getCurrentUILanguage"),
               NanNew<v8::FunctionTemplate>(
                   GetCurrentUILanguage)->GetFunction());
  exports->Set(NanNew("getCurrentGameInstallDir"),
               NanNew<v8::FunctionTemplate>(
                   GetCurrentGameInstallDir)->GetFunction());
  exports->Set(NanNew("getNumberOfPlayers"),
               NanNew<v8::FunctionTemplate>(GetNumberOfPlayers)->GetFunction());
  // WorkShop related APIs
  exports->Set(NanNew("fileShare"),
               NanNew<v8::FunctionTemplate>(FileShare)->GetFunction());
  // Utils related APIs.
  utils::InitUtilsObject(exports);
}

}  // namespace

#ifdef _WIN32
	NODE_MODULE(greenworks_win, init)
#elif __APPLE__
	NODE_MODULE(greenworks_osx, init)
#elif __linux__
  #if __x86_64__ || __ppc64__
    NODE_MODULE(greenworks_linux64, init)
  #else
    NODE_MODULE(greenworks_linux32, init)
  #endif
#endif