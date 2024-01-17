#include "glad/glad.h"
#include <bot.hxx>
#include <commands.hxx>
#include <compatibility.hxx>
#include <dpp.h>

constexpr uint32_t operator""_hash(const char* str, size_t)
{
    return hydra::str_hash(str);
}

Bot::Bot(std::shared_ptr<hydra::EmulatorWrapper> emulator, GLuint fbo, const std::string& token,
         int frames_pressed, int frames_released)
    : emulator(emulator), fbo(fbo),
      token(token.c_str()) // dpp doesn't like when the string has a bunch of extra \0s
      ,
      frames_pressed(frames_pressed), frames_released(frames_released)
{
    start();
}

void Bot::start()
{
    native_size = emulator->shell->getNativeSize();

    dpp::cluster bot(this->token, dpp::i_default_intents | dpp::i_message_content);
    bot.on_log(dpp::utility::cout_logger());
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>())
        {
            std::cout << "Registering commands" << std::endl;
            bot.global_bulk_command_create(
                Commands::Get(bot.me.id), [](const dpp::confirmation_callback_t& callback) {
                    if (callback.is_error())
                    {
                        std::cout << "Failed to register commands: " << callback.http_info.body
                                  << std::endl;
                    }
                    else
                    {
                        std::cout << "Successfully registered commands" << std::endl;
                    }
                });
        }
    });

    bot.on_slashcommand([](const dpp::slashcommand_t& event) {
        switch (hydra::str_hash(event.command.get_command_name().c_str()))
        {
            case "ping"_hash:
            {
                event.reply("Pong!");
                break;
            }
            case "status"_hash:
            {
                // dpp::embed embed =
                //     dpp::embed()
                //         .set_title(state->emulator->GetInfo(hydra::InfoType::CoreName))
                //         .set_url(state->emulator->GetInfo(hydra::InfoType::Website))
                //         .set_description(state->emulator->GetInfo(hydra::InfoType::Description))
                //         .set_author(state->emulator->GetInfo(hydra::InfoType::Author),
                //                     "https://github.com/hydra-emu/hydra",
                //                     "attachment://icon.png")
                //         .set_thumbnail("attachment://icon.png");
                // dpp::message msg = dpp::message(event.command.channel_id, embed);
                // if (state->emulator->GetIcon().size() > 0)
                // {
                //     msg.add_file("icon.png",
                //                  std::string(state->emulator->GetIcon().begin(),
                //                              state->emulator->GetIcon().end()),
                //                  "image/png");
                // }
                // event.reply(msg);
                break;
            }
            case "screen"_hash:
            {
                dpp::message msg(event.command.channel_id, "");
                msg.add_component(dpp::component()
                                      .add_component(dpp::component()
                                                         .set_label("L")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_secondary)
                                                         .set_id("l_button"))
                                      .add_component(dpp::component()
                                                         .set_label("↑")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_primary)
                                                         .set_id("up_button"))
                                      .add_component(dpp::component()
                                                         .set_label("R")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_secondary)
                                                         .set_id("r_button"))
                                      .add_component(dpp::component()
                                                         .set_label("A")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_success)
                                                         .set_id("a_button"))
                                      .add_component(dpp::component()
                                                         .set_label("X")
                                                         .set_type(dpp::cot_button)
                                                         .set_style(dpp::cos_secondary)
                                                         .set_id("x_button")))
                    .add_component(dpp::component()
                                       .add_component(dpp::component()
                                                          .set_label("←")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_primary)
                                                          .set_id("left_button"))
                                       .add_component(dpp::component()
                                                          .set_label("↓")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_primary)
                                                          .set_id("down_button"))
                                       .add_component(dpp::component()
                                                          .set_label("→")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_primary)
                                                          .set_id("right_button"))
                                       .add_component(dpp::component()
                                                          .set_label("Y")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("y_button"))
                                       .add_component(dpp::component()
                                                          .set_label("B")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_danger)
                                                          .set_id("b_button")))
                    .add_component(dpp::component()
                                       .add_component(dpp::component()
                                                          .set_label("👆")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("touch_button"))
                                       .add_component(dpp::component()
                                                          .set_label("\u200b")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("spacer2")
                                                          .set_disabled(true))
                                       .add_component(dpp::component()
                                                          .set_label("\u200b")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("spacer1")
                                                          .set_disabled(true))
                                       .add_component(dpp::component()
                                                          .set_label("➕")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("start_button"))
                                       .add_component(dpp::component()
                                                          .set_label("➖")
                                                          .set_type(dpp::cot_button)
                                                          .set_style(dpp::cos_secondary)
                                                          .set_id("select_button")));
                event.reply(msg);
                break;
            }
        }
    });

    bot.start(dpp::st_return);
}

void Bot::runFrame()
{
    bool flip_y = false;
    if (emulator->shell->hasInterface(hydra::InterfaceType::IOpenGlRendered))
    {
        hydra::IOpenGlRendered* shell_gl = emulator->shell->asIOpenGlRendered();
        shell_gl->setFbo(fbo);
        flip_y = true;
    }
    if (emulator->shell->hasInterface(hydra::InterfaceType::IFrontendDriven))
    {
        hydra::IFrontendDriven* shell = emulator->shell->asIFrontendDriven();
        // If there's an active bot, it runs the frame instead of the frontend
        shell->runFrame();
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    std::vector<uint8_t> buffer;
    buffer.resize(native_size.width * native_size.height * 4);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadPixels(0, 0, native_size.width, native_size.height, GL_RGBA, GL_UNSIGNED_BYTE,
                 buffer.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    pushFrame(buffer.data(), native_size);
}

void Bot::pushFrame(void* data, hydra::Size size)
{
    frames.push_back(
        std::vector<uint8_t>((uint8_t*)data, (uint8_t*)data + size.width * size.height * 4));
}

Bot::~Bot() {}

// namespace fs = std::filesystem;

// GLuint fbo;
// void* get_proc_address;

// void init_gl()
// {
//     if (!glfwInit())
//         printf("glfwInit() failed\n");
//     // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     GLFWwindow* window = glfwCreateWindow(640, 480, "", nullptr, nullptr);
//     if (!window)
//         printf("glfwCreateWindow() failed\n");
//     glfwMakeContextCurrent(window);
//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//         printf("gladLoadGLLoader() failed\n");
//     get_proc_address = (void*)glfwGetProcAddress;

//     GLuint texture;
//     glGenTextures(1, &texture);
//     glBindTexture(GL_TEXTURE_2D, texture);
//     glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 400, 480);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//     glGenFramebuffers(1, &fbo);
//     glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
// }

// int32_t read_input_callback(uint32_t player, hydra::ButtonType button)
// {
//     return 0;
// }

// struct BotState
// {
//     BotState()
//     {
//         init_gl();
//         std::string core_path = Settings::Get("core_path");
//         emulator = hydra::EmulatorFactory::Create(fs::path(core_path) / "libAlber.so");
//         width = emulator->shell->getNativeSize().width;
//         height = emulator->shell->getNativeSize().height;
//         buffer.resize(width * height * 4);

//         if (emulator->shell->hasInterface(hydra::InterfaceType::IOpenGlRendered))
//         {
//             auto gli = emulator->shell->asIOpenGlRendered();
//             gli->setGetProcAddress(get_proc_address);
//             gli->resetContext();
//             gli->setFbo(fbo);
//             emulator->shell->setOutputSize({width, height});
//         }

//         if (emulator->shell->hasInterface(hydra::InterfaceType::IInput))
//         {
//             hydra::IInput* shell_input = emulator->shell->asIInput();
//             shell_input->setPollInputCallback([]() {});
//             shell_input->setCheckButtonCallback(read_input_callback);
//         }

//         emulator->LoadGame("/home/offtkp/Roms/3DS/zelda.3ds");
//     }

//     const std::vector<uint8_t>& get_frame()
//     {
//         glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//         glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

//         for (uint32_t y = 0; y < height / 2; y++)
//         {
//             for (uint32_t x = 0; x < width; x++)
//             {
//                 std::swap(buffer[(y * width + x) * 4 + 0],
//                           buffer[((height - 1 - y) * width + x) * 4 + 0]);
//                 std::swap(buffer[(y * width + x) * 4 + 1],
//                           buffer[((height - 1 - y) * width + x) * 4 + 1]);
//                 std::swap(buffer[(y * width + x) * 4 + 2],
//                           buffer[((height - 1 - y) * width + x) * 4 + 2]);
//                 std::swap(buffer[(y * width + x) * 4 + 3],
//                           buffer[((height - 1 - y) * width + x) * 4 + 3]);
//             }
//         }
//         return buffer;
//     }

//     uint32_t width, height;
//     std::shared_ptr<hydra::EmulatorWrapper> emulator;
//     std::vector<uint8_t> buffer;
// };

// std::unique_ptr<BotState> state;

int bot_main()
{
    return 0;
}

//     std::string token = Settings::Get("bot_token");

//     if (token.empty())
//     {
//         token = std::string(std::getenv("BOT_TOKEN"));
//         Settings::Set("bot_token", token);
//     }

//     if (token.empty())
//     {
//         std::cerr << "No bot token found. Please set the BOT_TOKEN environment variable."
//                   << std::endl;
//         return 1;
//     }

//     std::string core_path = Settings::Get("core_path");

//     if (core_path.empty())
//     {
//         core_path = std::string(std::getenv("CORE_PATH"));
//         Settings::Set("core_path", core_path);
//     }

//     if (core_path.empty())
//     {
//         std::cerr << "No core path found. Please set the CORE_PATH environment variable."
//                   << std::endl;
//         return 1;
//     }

//     state = std::make_unique<BotState>();

//     bot.on_message_create([&bot](const dpp::message_create_t& event) {
//         /* See if the message contains the phrase we want to check for.
//          * If there's at least a single match, we reply and say it's not allowed.
//          */
//         printf("Got message: %s\n", event.msg.content.c_str());
//         if (event.msg.content.find("I hate pandas") != std::string::npos)
//         {
//             std::string fake_ip = std::to_string(rand() % 255) + "." +
//                                   std::to_string(rand() % 255) + "." +
//                                   std::to_string(rand() % 255) + "." + std::to_string(rand() %
//                                   255);
//             event.reply("That is not allowed here. Please, mind your language! I will now dox
//             your "
//                         "ip for saying this. Your ip is:" +
//                             fake_ip,
//                         true);
//         }
//     });

//     bot.start(dpp::st_wait);
//     return 0;
// }