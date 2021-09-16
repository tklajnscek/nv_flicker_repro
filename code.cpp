
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

///////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <math.h> // sin, cos for rotation

///////////////////////////////////////////////////////////////////////////////////////////////////

#define TITLE "Minimal D3D11 by d7samurai"

LRESULT CALLBACK WndProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch (nMsg)
    {
    case WM_CLOSE:
        // Sends us a WM_DESTROY
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        // Send a WM_QUIT to the main loop, ends the loop and returns from WinMain
        PostQuitMessage(0);
        return 0;

    default:
        // Let Windows take care of the other messages
        return DefWindowProc(hWnd, nMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    WNDCLASSEXA wndClassEx = { sizeof(wndClassEx) };
    wndClassEx.lpfnWndProc   = WndProc;
    wndClassEx.lpszClassName = TITLE;

    RegisterClassExA(&wndClassEx);

    HWND window = CreateWindowExA(0, TITLE, TITLE, WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    
    ID3D11Device* baseDevice;
    ID3D11DeviceContext* baseDeviceContext;

    D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3D11Device1* device;

    baseDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&device));

    ID3D11DeviceContext1* deviceContext;

    baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&deviceContext));

    ///////////////////////////////////////////////////////////////////////////////////////////////

    IDXGIDevice1* dxgiDevice;

    device->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&dxgiDevice));

    IDXGIAdapter* dxgiAdapter;

    dxgiDevice->GetAdapter(&dxgiAdapter);

    IDXGIFactory2* dxgiFactory;

    dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));

    ///////////////////////////////////////////////////////////////////////////////////////////////

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width              = 0; // use window width
    swapChainDesc.Height             = 0; // use window height
    swapChainDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    swapChainDesc.Stereo             = FALSE;
    swapChainDesc.SampleDesc.Count   = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount        = 2;
    swapChainDesc.Scaling            = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags              = 0;

    IDXGISwapChain1* swapChain;

    dxgiFactory->CreateSwapChainForHwnd(device, window, &swapChainDesc, nullptr, nullptr, &swapChain);

    
    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3D11Texture2D* frameBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&frameBuffer));

    ID3D11RenderTargetView* frameBufferView;
    device->CreateRenderTargetView(frameBuffer, nullptr, &frameBufferView);

    D3D11_TEXTURE2D_DESC backBufferDesc;
    frameBuffer->GetDesc(&backBufferDesc);


    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3DBlob* vsBlob;
    D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, nullptr);

    ID3D11VertexShader* vertexShader;

    device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);

    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = // float3 position
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    ID3D11InputLayout* inputLayout;

    device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3DBlob* psBlob;

    D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, nullptr);

    ID3D11PixelShader* pixelShader;

    device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    D3D11_RASTERIZER_DESC1 rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;

    ID3D11RasterizerState1* rasterizerState;

    device->CreateRasterizerState1(&rasterizerDesc, &rasterizerState);

 
    ///////////////////////////////////////////////////////////////////////////////////////////////

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable    = FALSE;
    ID3D11DepthStencilState* depthStencilState;

    device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);  

    ///////////////////////////////////////////////////////////////////////////////////////////////

    float VertexData[] = {
           -1.0f, -1.0f, 1.0f,
            1.f,  -1.0f, 1.0f,
            1.0f,  1.0f, 1.0f,
           -1.0f,  1.0f, 1.0f,
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(VertexData);
    vertexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = { VertexData };

    ID3D11Buffer* vertexBuffer;

    device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

    ///////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int IndexData[] = {
            0, 1, 2, 2, 3, 0
    };

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(IndexData);
    indexBufferDesc.Usage     = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = { IndexData };

    ID3D11Buffer* indexBuffer;

    device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer); 
   
    ///////////////////////////////////////////////////////////////////////////////////////////////

    UINT stride = 12;
    UINT offset = 0;

    D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (float)backBufferDesc.Width, (float)backBufferDesc.Height, 0.0f, 1.0f };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    while (true)
    {
        MSG msg;

        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                return 0;
            DispatchMessageA(&msg);
        }

        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        deviceContext->IASetInputLayout(inputLayout);
        deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
        deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
        deviceContext->VSSetShader(vertexShader, nullptr, 0);
        deviceContext->PSSetShader(pixelShader, nullptr, 0);
        deviceContext->RSSetViewports(1, &viewport);
        deviceContext->RSSetState(rasterizerState);
        deviceContext->OMSetRenderTargets(1, &frameBufferView, nullptr);
        deviceContext->OMSetDepthStencilState(depthStencilState, 0);

        ///////////////////////////////////////////////////////////////////////////////////////////

        deviceContext->DrawIndexed(ARRAYSIZE(IndexData), 0, 0);

        ///////////////////////////////////////////////////////////////////////////////////////////

        swapChain->Present(1, 0);

        SleepEx(100, FALSE);
    }
}