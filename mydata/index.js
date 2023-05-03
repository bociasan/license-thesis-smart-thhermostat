function getEl(id){
return document.getElementById(id)
}

const SET_TARGET = {
    type: 'set',
    value: 22,
    element: 'target'
}

const SET_MODE = {
    type: 'set',
    value: 1,
    element: 'mode'
}

const SET_STATUS = {
    type: 'set',
    value: 0,
    element: 'status'
}

const setFunc = (action) => {
    console.log(action.value)
    getEl(action.element).innerHTML = action.value
}

const getFunc = (action) => {

}

const toggleFunc = (element) => {
    switch (element){
        case 'mode':
            getEl(element).innerHTML = getEl(element).innerHTML === 'Auto'? 'Manual' : 'Auto'
            break;
        case 'status':
            getEl(element).innerHTML = getEl(element).innerHTML === 'Off'? 'On' : 'Off'
            break;
    }
}
const parseFunc = (action) => {
    switch (action.type){
        case 'set':
            setFunc(action)
            break;
        case 'get':
            getFunc(action)
            break;

    }
}

parseFunc(SET_TARGET)
// parseFunc(SET_MODE)
// parseFunc(SET_STATUS)

getEl('navbar-title').onclick = () => {alert("This is thermostat project :))")}
getEl('mode').onclick = () => {toggleFunc('mode')}
getEl('status').onclick = () => {toggleFunc('status')}

