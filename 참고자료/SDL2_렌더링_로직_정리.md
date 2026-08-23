# SDL2 렌더링 로직 전수 조사

이 문서는 `dk_client` 저장소 전체에서 SDL2(및 SDL_ttf)를 사용해 실제로 화면에 그림을 그리는 로직을 조사해 정리한 것이다. 코드 수정 없이 조사만 수행했다.

## 1. 개요

렌더링 파이프라인은 두 계층으로 나뉜다.

1. **소프트웨어 합성 계층 (SpriteLib, `SDL_Surface` 기반)** — 배경 타일, 스프라이트, 텍스트(폰트 글리프)를 매 프레임 CPU 메모리 상의 `SDL_Surface`(`g_pBack`, `g_pLast` 등 `CSpriteSurface`가 감싸고 있음)에 `SDL_BlitSurface`/`SDL_FillRect`/직접 픽셀 복사(RLE 디코딩 등)로 합성한다. 원래 DirectDraw 기반 구조(백버퍼/프론트버퍼 blit)를 그대로 흉내 낸 것이다.
2. **GPU 출력 계층 (`SDL_Renderer`)** — 한 프레임의 합성이 끝난 `g_pBack` 서페이스 전체를 SDL2 텍스처 하나로 업로드해서 `SDL_Renderer`로 화면에 `RenderCopy` + `RenderPresent` 한다. 게임 로직은 `SDL_Renderer`를 직접 건드리지 않고, `CSDLGraphics::Flip()` 또는 `SDLMain.cpp`의 메인 루프가 이 업로드/출력을 대행한다.

즉 실질적인 "그리기"는 거의 전부 SW 서페이스 blit이고, `SDL_Renderer`는 완성된 프레임 하나를 화면에 올리는 최종 프리젠테이션 용도로만 쓰인다. 텍스트도 예외가 아니라, `SDL_ttf`로 글리프를 렌더링한 뒤 SpriteLib의 스프라이트로 변환해서 같은 SW 합성 경로에 태운다.

## 2. 렌더러/윈도우 초기화

렌더러/윈도우 생성 경로가 **두 곳** 존재한다 (§8 참고).

- **`Client/SDLMain.cpp` `InitApp()` (line 105~185)**
  - `SDL_CreateWindow()` (line 139) — 창 크기/전체화면 여부는 커맨드라인 인자(`0000000001`~`0000000004`)로 결정.
  - `SDL_CreateRenderer(..., SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)` (line 153)
  - `SDL_SetRenderDrawColor(g_pSDLRenderer, 0,0,0,255)` (line 165) — `RenderClear` 시 검은색으로 지우기 위함
  - `SDL_ShowCursor(0)` (line 170)
  - `spritectl_init()` 호출로 SpriteLib SDL 백엔드(SW 서페이스 계층) 초기화
  - 종료 시 `CleanupSDL()` (line 192~203): `SDL_DestroyRenderer` → `SDL_DestroyWindow` → `spritectl_shutdown()`

- **`Client/DXLib/CDirectDraw.cpp` `CSDLGraphics::Init()` (line 68~98)**
  - 이미 만들어진 Win32 `HWND`를 `SDL_CreateWindowFrom()` (line 77)으로 감싼다 (새 창을 만들지 않음).
  - `SDL_CreateRenderer(m_pSDLWindow, -1, SDL_RENDERER_ACCELERATED)` (line 83), 실패 시 플래그 없는 `SDL_CreateRenderer` (line 86)로 폴백.
  - `ReleaseAll()` (line 106~121)에서 `SDL_DestroyRenderer` → `SDL_DestroyWindow`.
  - 이 렌더러는 `CSDLGraphics` 클래스의 **static 멤버** `m_pSDLRenderer`에 저장되며, `SDLMain.cpp`의 전역 `g_pSDLRenderer`와는 별개의 인스턴스다.

- **`spritectl_init()` / `spritectl_shutdown()`** (`SpriteLib/SpriteLibBackendSDL.cpp` line 28~56) — `SDL_Init(SDL_INIT_VIDEO)` / `SDL_Quit()`. `SDL_Renderer` 자체는 만들지 않고, SW 서페이스 계층에서 필요한 SDL 비디오 서브시스템만 초기화한다.

## 3. 프레임별 렌더링 흐름

두 가지 출력 경로가 공존한다.

### 3-1. `SDLMain.cpp` 메인 루프 (line 469~519, `main()` 안)

매 프레임 다음 순서로 실행:

1. `dxlib_input_update()` — SDL 이벤트 처리 (입력, 본 문서 범위 아님)
2. `SDL_RenderClear(g_pSDLRenderer)` (line 483) — 이전 프레임 잔상 방지용 명시적 클리어
3. `g_pUpdate->Update()` (line 488) — 게임 로직 갱신 + `g_pBack`/`g_pLast` SW 서페이스에 스프라이트/텍스트 합성 (실제 "그리기"는 대부분 여기서 SpriteLib blit 함수로 수행됨, §6 참고)
4. FPS 측정 (line 492~501) — 렌더링과 무관
5. `g_pBack->Blt(&origin, g_pLast, NULL)` (line 507) — UI가 그려진 `g_pLast`를 `g_pBack`에 복사해 최종 프레임 합성
6. `g_pBack->GetBackendSurface()` → `spritectl_present_surface(backend_surface, g_pSDLRenderer)` (line 511~514) — SW 서페이스를 GPU 텍스처로 업로드 후 `RenderCopy`
7. `SDL_RenderPresent(g_pSDLRenderer)` (line 517)
8. `g_FrameCount++` (line 518)

### 3-2. `CSDLGraphics::Flip()` 경로 (`Client/CSDLGraphicsFlip.cpp`)

레거시 `WinMain`/`Client.cpp` 계열 코드에서 여전히 프레임 종료 시점마다 직접 호출하는 함수. `CDirectDraw.cpp`가 `SPRITELIB_BACKEND_SDL` 없이 별도 라이브러리로 빌드되어 `CSpriteSurface`의 전체 정의를 가져올 수 없기 때문에, `Flip()` 구현만 `DarkEden.exe` 쪽 소스(`Client_MAIN_SOURCES`)로 분리되어 있다.

```
SDL_RenderClear(m_pSDLRenderer)
  → g_pBack->GetBackendSurface()
  → spritectl_present_surface(backendSurface, m_pSDLRenderer)
  → SDL_RenderPresent(m_pSDLRenderer)
```

호출 지점이 매우 많다 (전부 "한 프레임을 화면에 낸다"는 동일한 의도): `Client.cpp:4302`, `CGameUpdate.cpp:5794`, `CGameUpdate.cpp:6730`, `CWaitUIUpdate.cpp:1027`, `GameInit.cpp:388`, `GameInit.cpp:883`, `GameMain.cpp:747/1007/4836/5047/5103/5125`, `GameUI.cpp:2937`, `UIMessageManager.cpp:322/325/2190`. `MTopView.cpp:874`는 주석 처리되어 죽은 코드.

> `SDLMain.cpp` 루프와 `CSDLGraphics::Flip()`은 서로 다른 `SDL_Renderer` 인스턴스(`g_pSDLRenderer` vs `CSDLGraphics::m_pSDLRenderer`)를 대상으로 하는 동일한 패턴의 출력 코드다. 실제 실행 중에는 빌드 진입점(`SDLMain.cpp`의 `main()`인지, 레거시 `WinMain` 경로인지)에 따라 둘 중 하나만 의미 있게 동작할 것으로 보이나, 두 경로가 동시에 살아있는 상태이므로 §8에서 별도로 짚는다.

## 4. 텍스처 관리

GPU 텍스처(`SDL_Texture`)를 다루는 곳은 사실상 **`spritectl_present_surface()`** (`SpriteLib/SpriteLibBackendSDL.cpp` line 1297~1364) 한 곳뿐이다. SW 서페이스 → 화면 출력을 담당하는 유일한 지점이기 때문.

- **현재(uncommitted 변경 반영) 구조**: `surface->texture`에 스트리밍 텍스처(`SDL_TEXTUREACCESS_STREAMING`)를 캐싱해두고, 렌더러가 바뀌지 않는 한 재사용한다 (`SDL_CreateTexture` line 1328, `SDL_UpdateTexture` line 1343, `SDL_RenderCopy` line 1344). 렌더러가 바뀌면 기존 텍스처를 `SDL_DestroyTexture` (line 1323) 후 재생성.
  - 이 스트리밍 텍스처 생성이 실패하거나 픽셀 포맷을 렌더러가 못 받아들이면 `SDL_CreateTextureFromSurface` (line 1353) + `SDL_RenderCopy` (line 1359) + `SDL_DestroyTexture` (line 1363)로 매 프레임 텍스처를 새로 만들고 버리는 폴백 경로로 빠진다. (예전에는 이 폴백 경로가 유일한 경로였고, 매 프레임 텍스처 생성/파괴로 GPU/드라이버 측 메모리 churn을 유발했던 것을 최근 스트리밍 텍스처 재사용 구조로 고쳤다 — 상세 배경은 이전 대화 참고.)
- 그 외 `SDL_DestroyTexture` 호출이 `spritectl_destroy_surface()` (line 180, surface 파괴 시 함께 정리) 에도 있음.
- `SDL_UpdateTexture`/`SDL_LockTexture` 등 텍스처를 프레임마다 갱신하는 다른 지점은 없음 — 텍스처 계층은 이 한 곳에 집중되어 있다.

## 5. 텍스트(폰트) 렌더링 경로

`Client/TextSystem/TextBackendSDL.cpp`가 `SDL_ttf`를 사용하는 유일한 곳이다.

1. **폰트 로드**: `TTF_OpenFont()` (line 122), 소멸자에서 `TTF_CloseFont()` (line 57)로 정리. 폰트 파일 후보 목록을 순회하며 로드.
2. **글리프 래스터화**: `GetGlyph()` (line 199~) 에서 코드포인트+색상+폰트를 키로 하는 `m_glyphs` 캐시(`unordered_map<GlyphKey, Glyph>`)를 먼저 조회하고, 캐시 미스일 때만:
   - `TTF_RenderUTF8_Blended(ttf, utf8.c_str(), sdlColor)` (line 232)로 `SDL_Surface`를 얻고,
   - 필요 시 `SDL_ConvertSurfaceFormat(..., SDL_PIXELFORMAT_RGBA32, 0)` (line 238)로 포맷 통일,
   - `spritectl_create_sprite()` (line 271)로 SpriteLib 스프라이트로 변환해 `glyph.handle`에 저장,
   - 임시로 만든 `SDL_Surface`는 `SDL_FreeSurface` (line 239, 278)로 즉시 해제.
3. 캐시된 글리프(SpriteLib 스프라이트)는 이후 일반 스프라이트와 동일하게 `spritectl_blt_sprite*` 계열로 `CSpriteSurface`(`g_pLast` 등)에 blit된다 (`TextService.cpp` line 453 부근에서 `g_pLast` 대상 렌더 타깃 확인).
4. **글리프 메트릭 fallback**: `GetGlyphMetrics()` (line 158~197)도 메트릭 API가 실패하면 `TTF_RenderUTF8_Blended` (line 177)로 한 번 더 렌더링해 크기를 추정하고 즉시 `SDL_FreeSurface` (line 185).
5. **렌더 타깃 어댑터**: `TextSystem/RenderTargetSpriteSurface.cpp`의 `SpriteSurfaceRenderTarget`이 `CSpriteSurface`를 텍스트 시스템이 그릴 수 있는 대상으로 노출 (`GetNative()`가 내부적으로 `CSpriteSurface::GetBackendSurface()` 반환). 여기서 SDL API를 직접 호출하지는 않음.

글리프는 캐싱되므로 같은 문자/색상 조합에 대해 매 프레임 `TTF_RenderUTF8_Blended`가 다시 호출되지는 않는다.

## 6. 스프라이트 렌더링 경로

전부 `Client/SpriteLib/SpriteLibBackendSDL.cpp`에 구현되어 있고, `CSpriteSurface_SDL.cpp`(`CSpriteSurface`의 SDL 백엔드 구현, SPRITELIB_BACKEND_SDL일 때만 컴파일)가 이 API들을 감싸서 게임 로직에 노출한다.

- **서페이스 생성/파괴**: `spritectl_create_surface()`(line 134, 내부적으로 `spritectl_sdl_create_surface()`→`SDL_CreateRGBSurface` line 69/85/101/108) / `spritectl_destroy_surface()`(line 168, `SDL_FreeSurface` line 183).
- **락/언락**: `spritectl_lock_surface()`/`spritectl_unlock_surface()` (line 189, 212) — 내부적으로 `SDL_LockSurface`/`SDL_UnlockSurface`(line 195, 217 등, 총 6곳: line 195, 217, 384, 402, 525, 635, 696, 699, 733, 811, 814).
- **단색 채우기**: `spritectl_clear_surface()` (line 223) → `SDL_FillRect` (line 236). `CSpriteSurface_SDL.cpp:219`도 별도로 `SDL_FillRect` 호출.
- **스프라이트 blit (핵심 draw 경로, 매 프레임 다수 호출)**:
  - `spritectl_blt_sprite_rle()` (line 371~) — RLE 압축 스프라이트를 대상 서페이스에 직접 픽셀 단위로 디코딩/합성 (SDL blit API가 아니라 수동 픽셀 복사).
  - `spritectl_blt_sprite()` (line 603~) — 알파/그림자 등 효과가 있는 스프라이트를 `SDL_CreateRGBSurface`(line 646)로 임시 표면을 만든 뒤 `SDL_BlitSurface`(line 720)로 합성, 끝나면 `SDL_FreeSurface`(line 728). **매 호출마다 임시 서페이스를 새로 만들고 버리는 구조** — 스프라이트 1개 그릴 때마다 힙 할당 발생.
  - `spritectl_blt_sprite_scaled()` (line 741~) — 확대/축소 스프라이트. 역시 임시 서페이스 2개(원본+스케일본, line 761/817)를 만들어 `SDL_BlitSurface`(line 834) 후 둘 다 해제(line 842/843).
  - `spritectl_blt_surface()` (line 848~) — 서페이스 대 서페이스 직접 복사, `SDL_BlitSurface` (line 884).
- **RLE 스프라이트 생성/스캔라인 설정**: `spritectl_create_sprite_rle()`(line 531), `spritectl_sprite_set_scanline_rle()`(line 572) — SDL API 직접 호출은 없고 자체 버퍼 조작.
- **팩(pack) 파일 로드**: `spritectl_load_pack()`(line 896), `spritectl_load_sprite_from_file()`(line 1172) — 디스크에서 스프라이트 데이터를 읽어 위 생성 함수들로 변환. 렌더링 자체는 아니지만 렌더링 데이터의 입구.
- **최종 프리젠테이션**: `spritectl_present_surface()` (line 1297) — §3, §4 참고.

게임 로직 쪽에서 `ClientFunction.cpp:475`가 `gpC_base->m_p_DDSurface_back->GetBackendSurface()`로 백엔드 서페이스 핸들을 직접 꺼내 쓰는 지점이 있음 (구체적 사용처는 별도 확인 필요).

## 7. SDL2 렌더링 API 호출 전수 목록

| API | 위치 (파일:라인) | 용도 |
|---|---|---|
| `SDL_CreateWindow` | `SDLMain.cpp:139` | 메인 게임 창 생성 |
| `SDL_CreateWindowFrom` | `DXLib/CDirectDraw.cpp:77` | 기존 HWND를 SDL 창으로 래핑 (레거시 경로) |
| `SDL_DestroyWindow` | `SDLMain.cpp:199`, `DXLib/CDirectDraw.cpp:118` | 창 해제 |
| `SDL_CreateRenderer` | `SDLMain.cpp:153`, `DXLib/CDirectDraw.cpp:83,86` | 렌더러 생성 (두 경로) |
| `SDL_DestroyRenderer` | `SDLMain.cpp:195`, `DXLib/CDirectDraw.cpp:110`, `SpriteLibBackendSDL.cpp:50` | 렌더러 해제 |
| `SDL_SetRenderDrawColor` | `SDLMain.cpp:165`, `DXLib/CDirectDraw.cpp:91` | Clear 색상(검정) 지정 |
| `SDL_ShowCursor` | `SDLMain.cpp:170` | 커서 숨김 |
| `SDL_RenderClear` | `SDLMain.cpp:483`, `CSDLGraphicsFlip.cpp:31` | 프레임 시작 시 백버퍼 클리어 |
| `SDL_RenderCopy` | `SpriteLibBackendSDL.cpp:1344,1359` | 텍스처 → 화면 출력 |
| `SDL_RenderPresent` | `SDLMain.cpp:517`, `CSDLGraphicsFlip.cpp:42` | 화면에 프레젠트 |
| `SDL_RenderSetClipRect` (주석 언급) | `SpriteLib/CSpriteSurface_SDL.cpp:514` | 클리핑 관련 주석, 실제 SDL 렌더러 clip 호출은 아님(서페이스 clip) |
| `SDL_CreateTexture` | `SpriteLibBackendSDL.cpp:1328` | 스트리밍 텍스처 생성/캐싱 |
| `SDL_CreateTextureFromSurface` | `SpriteLibBackendSDL.cpp:1353` | 폴백 경로 텍스처 생성 |
| `SDL_UpdateTexture` | `SpriteLibBackendSDL.cpp:1343` | 캐싱된 텍스처에 픽셀 갱신 |
| `SDL_DestroyTexture` | `SpriteLibBackendSDL.cpp:180,1323,1363` | 텍스처 해제 |
| `SDL_CreateRGBSurface` | `SpriteLibBackendSDL.cpp:69,85,101,108,646,761,817` | SW 서페이스/임시 blit 버퍼 생성 |
| `SDL_FreeSurface` | `SpriteLibBackendSDL.cpp:183,728,820,842,843`, `TextBackendSDL.cpp:185,239,278`, `UtilityFunction.cpp:1007,1016,1017,1038,1039` | 서페이스 해제 |
| `SDL_LockSurface` / `SDL_UnlockSurface` | `SpriteLibBackendSDL.cpp:195/217, 384/402, 525, 635, 696/699, 733, 811/814` | 픽셀 버퍼 접근 |
| `SDL_FillRect` | `SpriteLibBackendSDL.cpp:236`, `CSpriteSurface_SDL.cpp:219` | 단색 채우기 |
| `SDL_BlitSurface` | `SpriteLibBackendSDL.cpp:720,834,884` | SW 서페이스 간 합성 (스프라이트 그리기 핵심) |
| `SDL_ConvertSurfaceFormat` | `TextBackendSDL.cpp:238`, `UtilityFunction.cpp:1005` | 픽셀 포맷 변환 (텍스트 RGBA 통일, 이미지 로드 시 RGB565 변환) |
| `TTF_OpenFont` / `TTF_CloseFont` | `TextBackendSDL.cpp:122` / `57` | 폰트 로드/해제 |
| `TTF_RenderUTF8_Blended` | `TextBackendSDL.cpp:177,232` | 글리프 래스터화 |
| `TTF_GlyphMetrics` / `TTF_FontAscent` / `TTF_FontDescent` | `TextBackendSDL.cpp` 다수 | 폰트 메트릭 조회 (렌더링은 아니지만 텍스트 레이아웃에 필수) |
| `SDL_GetPixelFormatName` / `SDL_GetError` | 다수 | 디버그 로그용, 렌더링 동작 자체는 아님 |

## 8. 참고사항 / 향후 정리 시 주의할 점

- **렌더러/윈도우 생성 경로 중복**: `SDLMain.cpp::InitApp()`(새 창 직접 생성, `g_pSDLRenderer`)과 `DXLib/CDirectDraw.cpp::CSDLGraphics::Init()`(기존 HWND 래핑, `CSDLGraphics::m_pSDLRenderer` static)가 서로 다른 `SDL_Window`/`SDL_Renderer` 인스턴스를 만든다. `CSDLGraphics::Flip()`은 후자를, `SDLMain.cpp` 메인 루프는 전자를 사용한다. 두 경로가 실제로 동시에 살아서 각자 창을 만드는 것인지, 아니면 빌드 진입점에 따라 한쪽만 쓰이는 죽은 코드인지는 `main()`/`WinMain()` 어느 쪽이 실제 빌드에 링크되는지 확인이 필요하다. 정리 대상 1순위.
- **`spritectl_blt_sprite()` / `spritectl_blt_sprite_scaled()`의 임시 서페이스 할당**: 알파 블렌딩이 필요한 스프라이트 1개를 그릴 때마다 `SDL_CreateRGBSurface`로 임시 버퍼를 새로 만들고 그린 뒤 바로 `SDL_FreeSurface`하는 구조다. 매 프레임 스프라이트가 수십~수백 개 그려진다면 힙 할당/해제가 그만큼 반복된다 — RLE 경로(`spritectl_blt_sprite_rle`)는 이런 임시 할당이 없어 대조적이다. 성능이나 힙 churn이 문제가 되면 이 함수들을 우선 검토할 만하다.
- **`spritectl_present_surface()`의 폴백 경로**: 렌더러가 스트리밍 텍스처의 픽셀 포맷을 지원하지 않을 때만 타는 `SDL_CreateTextureFromSurface` 폴백은 매 프레임 텍스처를 새로 만들고 버린다. 정상 경로(스트리밍 텍스처 재사용)가 항상 성공하는지 로그로 확인해두는 게 좋다.
- **디버그 로그가 렌더링 경로에 섞여 있음**: `SpriteLibBackendSDL.cpp`의 서페이스 생성 함수들(line 62~118)과 예전 텍스처 생성 코드에 `fprintf(stderr, ...)` 디버그 출력이 정적 카운터로 앞 몇 번만 찍히도록 남아있다. 문제 진단이 끝나면 정리 대상.
- **`MTopView.cpp:874`**: `CSDLGraphics::Flip()` 호출이 주석 처리된 채 남아있음 — 죽은 코드, 삭제 후보.
- **`ClientFunction.cpp:475`**: `GetBackendSurface()`를 게임 로직에서 직접 캐스팅해 사용하는 지점. 이 문서에서는 존재만 확인했고 구체적 용도는 별도 조사가 필요.
